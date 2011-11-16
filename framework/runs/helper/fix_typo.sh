#!/bin/sh
if [ "$1" = "" ] ; then
    echo "usage: ./fix_typo.sh [path to trace folder]"
    exit 1
fi

path=$1

if [ ! -d $path ]; then
    echo "trace folder \"$path\" does not exist"
    exit 1
fi

for trace_file in $path/*.out ; do

sed -E -e '{
N
s/Mispred_penalty_cond_br:([:blank:]*)([^\n]*)\nIndirect_MPKI/Mispred_penalty_ind_br:\1 \2\nIndirect_MPKI/
}' $trace_file | cat > temp ;

mv temp $trace_file

done

