#!/usr/bin/python

# This is the highest level script. It runs everything necessary
# such as calling the Makefile, doing parsing, etc

import os, sys, re, glob, random;

POPULATION = 4; # MUST BE DIVISIBLE BY 4
MAX_LINES = 60;
SEED = 983;
MAX_THREADS = 12;
MUTATION_RATE = 2; # Number of mutations per mating
NUM_ITER = 10;

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

# Setup
os.system("make java");
os.system("rm -rf predictors");
os.system("rm -f results");
os.system("echo '' > results");
os.system('mkdir predictors')

# First, compile and generate the initial batch
gen = "java -cp .:antlr-3.4-complete.jar BPLangProg gen predictors/iter_0 "+str(POPULATION)+" "+str(MAX_LINES)+" "+str(getSeed());
print gen;
os.system(gen);

for iteration in range(NUM_ITER):
    # Generate an executable for every predictor
    os.system("rm -rf bin/*");
    os.system("make -j"+str(MAX_THREADS)+" SRCDIR=predictors/iter_"+str(iteration));

    # Next, simulate every predictor (changedir to $PARSER/runs)
    os.chdir("runs");
    os.system("rm -rf results/*");
    os.system("./run.py "+str(MAX_THREADS));

    # Get the results, taken from cull.py (changedir to $PARSER/runs/results)
    os.chdir('results')
    results = glob.glob('*.result')
    predictors = []
    for predictor in results:
        trimmedName = predictor.rpartition('.')[0]
        #with open(predictor, 'r') as f:
        f = open(predictor, 'r')
        for line in f:
            if line.find('Average conditional MPPKI') >= 0 :
                number = line.rpartition(': ')[2].strip()
                break

        predictors.append((trimmedName, int(number)))

    predictors.sort(key=lambda predictor: predictor[1])
    
    for p in predictors:
        print p

    # Save the results (changedir to $PARSER)
    os.chdir('../..')
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
    
    # randomly merge predictors using the tournament method in Emer97
    matePred = emerSel(predictors);
    
    # For each pair, call the mating function twice 
    # (to preserve same population size)
    newIter = -1;
    os.system("mkdir predictors/iter_"+str(iteration+1));
    for pred in matePred:
        path = "predictors/iter_"+str(iteration);
        pred1 = path + "/" + pred[0]+"/bplang";
        pred2 = path + "/" + pred[1]+"/bplang";
        for numChildren in range(4):
            newIter = newIter+1;
            os.system("mkdir predictors/iter_"+str(iteration+1)+"/predictor_"+str(newIter));
            run = "java -cp .:antlr-3.4-complete.jar BPLangProg mate "+pred1+" "+pred2+" predictors/iter_"+str(iteration+1)+"/predictor_"+str(newIter)+" "+str(MUTATION_RATE)+" "+str(getSeed());
            print run;
            os.system(run);
        
