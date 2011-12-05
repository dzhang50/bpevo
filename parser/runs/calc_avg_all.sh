#!/bin/sh

cd helper

rm ../results/*.result

for predictor in ../results/* ; do
    ./calc_avg.sh $predictor
done

