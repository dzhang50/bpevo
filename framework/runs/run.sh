#!/bin/sh
if [ "$1" = "" ] ; then
    echo "usage: ./run.sh [predictor]"
    exit 1
fi

results=results
bin=bin
helper=helper

if [ ! -f ../$bin/$1 ] ; then
    echo "file \"../$bin/$1\" not found"
    exit 1
fi

mkdir -p $results/$1

for trace_file in ../traces/*.bz2 ; do
    trace_type=$(basename $trace_file)
    trace_type=`echo $trace_type | sed 's/\.[^.]*$//'`

    output_file=$results/$1/$trace_type.out
    ../$bin/$1 -t $trace_file > $output_file
    echo $trace_type result written to $output_file
done

cd $helper
./calc_avg.sh ../$results/$1
cd ..