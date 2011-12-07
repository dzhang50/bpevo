#!/usr/bin/python

import os
import glob
os.chdir('results')
results = glob.glob('*.result')
predictors = []
for predictor in results:
    trimmedName = predictor.rsplit('.')[0]
    #print trimmedName
    #with open(predictor, 'r') as f:
    f = open(predictor, 'r')
    for line in f:
        if line.find('Average conditional MPPKI') >= 0 :
            number = line.rsplit(':')[1].strip()
            break
        
    #print (trimmedName, int(number))
    if len(number) > 0:
        predictors.append((trimmedName, int(number)))
    else:
        print trimmedName, ' failed'

predictors.sort(key=lambda predictor: predictor[1])

#os.chdir('../..')
#os.system('mkdir -p next_gen')

for p in predictors:
    print p







