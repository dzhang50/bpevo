#!/usr/local/bin/python

# This is the highest level script. It runs everything necessary
# such as calling the Makefile, doing parsing, etc

import argparse, os, sys, re, glob, random;
from sets import Set
from shutil import copytree, move

parser = argparse.ArgumentParser(description='Paramters.');
parser.add_argument('name', help='name for results file');
parser.add_argument('-p', type=int, help='Population size (24)', default=24);
parser.add_argument('-l', type=int, help='Maximum lines per predictor (12)', default=12);
parser.add_argument('-i', type=int, help='Number of generations to run (10)', default=10);
parser.add_argument('-m', type=int, help='NUmber of mutations per mating (1)', default=1);
parser.add_argument('-mt', type=int, help='Maximum number of mutations (16)', default=16);
parser.add_argument('-st', type=int, help='Number of iterations where local min doesnt change (8)', default=8);
parser.add_argument('-s', type=int, help='Seed for reproducibility (981)', default=981);
parser.add_argument('-c', type=int, help='Use clustering (1)', default=1);
args=parser.parse_args();

POPULATION=args.p;
MAX_LINES=args.l;
SEED=args.s; 
CLUSTER_ENABLE=args.c;
STAGNATION_THRESHOLD=args.st; 
MUTATION_INIT=args.m;
MUTATION_THRESHOLD=args.mt;
NUM_ITER=args.i;
MAX_THREADS = min(12,POPULATION);
NAME=args.name;

# Function for getting the new random number seed
def getSeed():
    global SEED;
    tmp = SEED;
    SEED = SEED + 1;
    return tmp;

# Genetic algorithm selection function (SIZE MUST BE DIVISIBLE BY 4)
def emerSel(pred):
    matePred = [];
    print "Len pred: "+str(len(pred));
    # Make a copy, since we are going to modify the list
    myPred = list(pred);
    
    while(len(myPred) > 0):
        print "Len myPred: "+str(len(myPred));
        
        # Randomly select four predictors, choose the best for mating
        pred1 = myPred.pop(random.randint(0, len(myPred)-1));
        pred2 = myPred.pop(random.randint(0, len(myPred)-1));
        bestPred1 = "";
        if(pred1[1] < pred2[1]):
            bestPred1 = pred1[0];
        else:
            bestPred1 = pred2[0];
    
        
        pred1 = myPred.pop(random.randint(0, len(myPred)-1));
        if(len(myPred) != 1):
            pred2 = myPred.pop(random.randint(0, len(myPred)-1));
        else:
            pred2 = myPred.pop();

        bestPred2 = "";
        if(pred1[1] < pred2[1]):
            bestPred2 = pred1[0];
        else:
            bestPred2 = pred2[0];
        
        matePred.append((bestPred1, bestPred2));
    print matePred;
    return matePred;


# Generate executables, simulate predictors, return predictors sorted by fitness
def calcFitness(srcDir):
    global MAX_THREADS;
    # Generate an executable for every predictor
    os.system("rm -rf bin/*");
    os.system("make -j"+str(MAX_THREADS)+" SRCDIR="+srcDir);

    # Next, simulate every predictor (changedir to $PARSER/runs)
    os.chdir("runs");
    results = glob.glob('results/*')
    for predictor in results:
         if not os.path.basename(predictor).startswith("elite"):
             os.system("rm -rf " + predictor)
        
    os.system("./run.py "+str(MAX_THREADS));

    # Get the results, taken from cull.py (changedir to $PARSER/runs/results)
    os.chdir('results')
    results = glob.glob('*.result')
    predictors = []
    for predictor in results:
        trimmedName = predictor.rsplit('.')[0]
        #with open(predictor, 'r') as f:
        f = open(predictor, 'r')
        for line in f:
            if line.find('Average conditional MPPKI') >= 0 :
                number = line.rsplit(':')[1].strip()
                break

        predictors.append((trimmedName, int(number), int(number)))

    # Clustering avoidance algorithm
    count = dict();
    for x in range(len(predictors)):
        if(predictors[x][1] in count):
            count[predictors[x][1]] += 1;
        else:
            count[predictors[x][1]] = 1;
        newNumber = predictors[x][1] * count[predictors[x][1]];
        predictors[x] = (predictors[x][0], int(predictors[x][1]), int(newNumber));
    
    predictors.sort(key=lambda predictor: predictor[1])
    
    # return to $PARSER
    os.chdir('../..')
    
    return predictors;


def writeResults(predictors):
    os.system("mkdir results/");
    os.system("touch results/"+NAME);
    f = open("results/"+NAME, "w");
    for val in predictors:
        f.write(str(val[1])+"\n");
    f.close();


best = 100000;
lastBest = 0;
mutationRate = MUTATION_INIT;
def setMutationRate(predictors):
    global MUTATION_INIT, MUTATION_THRESHOLD, best, lastBest, mutationRate;

    if(predictors[0][1] < best):
        best = predictors[0][1];
        mutationRate = MUTATION_INIT;
    else:
        lastBest = lastBest + 1;

    if(lastBest > STAGNATION_THRESHOLD):
        if(mutationRate < MUTATION_THRESHOLD):
            mutationRate = mutationRate * 2;
            print "STAGNATION, SETTING MUTATION RATE = "+str(mutationRate);
            if(mutationRate > MUTATION_THRESHOLD):
                mutationRate = MUTATION_THRESHOLD;

# Setup
os.system("make java");
os.system("rm -rf predictors");
os.system("rm -rf runs/results/*");
os.system('mkdir predictors')

# First, compile and generate the initial batch
init = "java -cp .:antlr-3.4-complete.jar BPLangProg init predictors/iter_0 "+str(POPULATION)+" "+str(MAX_LINES)+" "+str(getSeed());
print init;
os.system(init);

iteration = 0;

while iteration < NUM_ITER:
#while true:
    predictors = calcFitness("predictors/iter_"+str(iteration));
    
    # Save the results
    writeResults(predictors);
    
    # Use seed predictors
    seedPredictors = calcFitness("seed");
    predictors.extend(seedPredictors);
    predictors.sort(key=lambda predictor: predictor[1]);
    
    # randomly merge predictors using the tournament method in Emer97
    matePred = emerSel(predictors);
    
    # Generate mutation rate based on last time new local min was found
    setMutationRate(predictors);

    # For each pair, call the mating function twice 
    # (to preserve same population size)
    newIter = -1;
    os.system("mkdir predictors/mateIter_"+str(iteration));
    for pred in matePred:
        path = "predictors/iter_"+str(iteration);
        pred1 = "";
        pred2 = "";
        if(pred[0].count("seed") == 0):
            pred1 = path + "/" + pred[0]+"/bplang";
        else:
            pred1 = "seed/" + pred[0]+"/bplang";
        if(pred[1].count("seed") == 0):
            pred2 = path + "/" + pred[1]+"/bplang";
        else:
            pred2 = "seed/" + pred[1]+"/bplang";

        for numChildren in range(2):
            newIter = newIter+1;
            os.system("mkdir predictors/mateIter_"+str(iteration)+"/matePredictor_"+str(newIter));
            run = "java -cp .:antlr-3.4-complete.jar BPLangProg mate "+pred1+" "+pred2+" predictors/mateIter_"+str(iteration)+"/matePredictor_"+str(newIter)+" "+str(mutationRate)+" "+str(getSeed());
            print run;
            os.system(run);

    # Calculate performance for new predictors
    matePredictors = calcFitness("predictors/mateIter_"+str(iteration));
    
    # Merge predictors
    predictors.extend(matePredictors);
    if(CLUSTER_ENABLE == 1):
        predictors.sort(key=lambda predictor: predictor[2]);
    else:
        predictors.sort(key=lambda predictor: predictor[1]);
    
    for p in predictors:
        print p;
        
    # Remove seed predictors
    i = 0;
    while(i < len(predictors)):
        if(predictors[i][0].count("seed") == 1):
            predictors.remove(predictors[i]);
        else:
            i=i+1;


    # Construct new generation
    os.system("mkdir predictors/iter_"+str(iteration+1));
    newIter = 0;
    for i in range(POPULATION):
        dest = " predictors/iter_"+str(iteration+1)+"/predictor_"+str(newIter);
        os.system("mkdir "+dest);
        if(predictors[i][0].count("mate") == 0):
            os.system("cp predictors/iter_"+str(iteration)+"/"+predictors[i][0]+"/*"+dest);
        else:
            os.system("cp predictors/mateIter_"+str(iteration)+"/"+predictors[i][0]+"/*"+dest);

        newIter = newIter + 1;
        

    iteration = iteration + 1;
