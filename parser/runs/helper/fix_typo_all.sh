#!/bin/sh

for trace_folder in ../results/*; do
    ./fix_typo.sh $trace_folder
done