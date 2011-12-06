#!/usr/bin/python

import os
import glob
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

os.chdir('../..')
os.system('mkdir -p next_gen')

for p in predictors:
    print p







