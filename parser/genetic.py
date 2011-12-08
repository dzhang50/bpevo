#!/usr/bin/python

# This is the highest level script. It runs everything necessary
# such as calling the Makefile, doing parsing, etc

import os, sys, re, glob, random;
from sets import Set
from shutil import copytree, move

POPULATION = 12; # MUST BE DIVISIBLE BY 4
ELITE_SIZE = 4
MAX_LINES = 6;
SEED = 983;
MAX_THREADS = min(12,POPULATION);
STAGNATION_THRESHOLD = 4; # Number of iterations where local min doesnt change
MUTATION_INIT = 2;  # Number of mutations per mating
MUTATION_THRESHOLD = 100; # Maximum number of mutations
NUM_ITER = 4;

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
os.system("rm -rf runs/results/*");
os.system("echo '' > results");
os.system('mkdir predictors')

# First, compile and generate the initial batch
init = "java -cp .:antlr-3.4-complete.jar BPLangProg init predictors/iter_0 "+str(POPULATION)+" "+str(MAX_LINES)+" "+str(getSeed());
print init;
os.system(init);

best = 100000;
lastBest = 0;
mutationRate = MUTATION_INIT;
iteration = 0;
current_elite = Set()
old_elite = Set()

while iteration < NUM_ITER:
#while true:
    # Generate an executable for every predictor
    os.system("rm -rf bin/*");
    os.system("make -j"+str(MAX_THREADS)+" SRCDIR=predictors/iter_"+str(iteration));

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

        predictors.append((trimmedName, int(number)))

    predictors.sort(key=lambda predictor: predictor[1])
    old_elite = current_elite.copy()
    current_elite.clear()
    for predictor in predictors[0:ELITE_SIZE]:
        current_elite.add(predictor[0])

    eliminatedElites = old_elite.difference(current_elite)
    newElites = current_elite.difference(old_elite)

    for p in predictors:
        print p

    print "Old elite: " + str(old_elite)
    print "New elite: " + str(current_elite)
    print "Eliminated elites: " + str(eliminatedElites)
    print "Added elites: " + str(newElites)

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
    matePred = emerSel(predictors[0:(len(predictors) - ELITE_SIZE)]);
    
    # Generate mutation rate based on last time new local min was found
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
            run = "java -cp .:antlr-3.4-complete.jar BPLangProg mate "+pred1+" "+pred2+" predictors/iter_"+str(iteration+1)+"/predictor_"+str(newIter)+" "+str(mutationRate)+" "+str(getSeed());
            print run;
            os.system(run);

    #os.system("rm -rf results/*");

    #rename and move C code
    os.chdir('predictors/iter_' + str(iteration))
    #for eliminated in eliminatedElites:
    #    move(eliminated, '../iter_' + str(iteration+1) + '/' eliminated.replace("elite_",""))

    for elite in current_elite:
        newName = "elite_gen_"+ str(iteration) +'_' + elite
        if not elite.startswith("elite_"):
            copytree(elite, '../iter_' + str(iteration+1) + '/' + newName)
        else:
            copytree(elite, '../iter_' + str(iteration+1) + '/' + elite)

    os.chdir('../../runs/results')
    #rename and remove result
    for eliminated in eliminatedElites:
        os.system('rm -fr ' + eliminated +'*')

    for addition in newElites:
        newName = "elite_gen_"+ str(iteration) +'_' + addition
        move(addition, newName)
        move(addition + '.result', newName + '.result')
        current_elite.remove(addition)
        current_elite.add(newName)
    
    #eliminatedElites = old_elite.difference(current_elite)
    #newElites = current_elite.difference(old_elite)

    os.chdir('../..')

    iteration = iteration + 1;
