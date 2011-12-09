#!/usr/bin/python

# This is the highest level script. It runs everything necessary
# such as calling the Makefile, doing parsing, etc

import os, sys, re, glob, random;
from sets import Set
from shutil import copytree, move

POPULATION = 24; # MUST BE DIVISIBLE BY 4
MAX_LINES = 12;
SEED = 981;
CLUSTER_ENABLE = 1;
MAX_THREADS = min(12,POPULATION);
STAGNATION_THRESHOLD = 8; # Number of iterations where local min doesnt change
MUTATION_INIT = 1;  # Number of mutations per mating
MUTATION_THRESHOLD = 16; # Maximum number of mutations
NUM_ITER = 100;

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
        
    #os.system("rm -rf results/*");
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
    os.system("cp results resultsBackup");
    f = open("results", "r");
    contents = f.readlines();
    f.close();

    print "len contents: "+str(len(contents));
    print contents;

    f = open("results", "w");
    # Init
    if(len(contents) == 1):
        for val in predictors:
            f.write(str(val[1])+"\n");
    # Append
    else:
        for x in range(len(predictors)):
            f.write(contents[x].rstrip()+", "+str(predictors[x][1])+"\n");
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
os.system("rm -f results");
os.system("rm -rf runs/results/*");
os.system("echo '' > results");
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
