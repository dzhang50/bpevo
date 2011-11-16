#!/bin/sh
if [ "$1" = "" ] ; then
    echo "usage: ./calc_avg.sh [path to trace folder]"
    exit 1
fi

path=$1

if [ ! -d $path ]; then
    echo "trace folder \"$path\" does not exist"
    exit 1
fi

sum=0
count=0

num_cond_br_sum=0
mispred_cond_br_sum=0
mispred_pen_cond_br_sum=0

for output_file in $path/*.out ; do
    
    mppki="$(awk -f cond_mppki.awk $output_file)"
    num_cond_br="$(awk -f num_cond_br.awk $output_file)"
    mispred_cond_br="$(awk -f mispred_cond_br.awk $output_file)"
    mispred_pen_cond_br="$(awk -f mispred_pen_cond_br.awk $output_file)"

    sum=$(echo "$sum + $mppki" | bc)
    num_cond_br_sum=$(echo "$num_cond_br_sum + $num_cond_br" | bc)
    mispred_cond_br_sum=$(echo "$mispred_cond_br_sum + $mispred_cond_br" | bc)
    mispred_pen_cond_br_sum=$(echo "$mispred_pen_cond_br_sum + $mispred_pen_cond_br" | bc)

    count=$((count + 1))
done

result=$(echo "$sum / $count" | bc)
accuracy=$(echo "scale=4;($mispred_cond_br_sum / $num_cond_br_sum)*100" | bc)
avg_pen_per_miss=$(echo "scale=4;($mispred_pen_cond_br_sum / $mispred_cond_br_sum)" | bc)

out=../results/$(basename $path).result

echo "Average conditional MPPKI                                : $result" | cat > $out
echo "Total number of conditional branches                     : $num_cond_br_sum" | cat >> $out
echo "Total number of conditional branches mispredicts         : $mispred_cond_br_sum" | cat >> $out
echo "Total number of conditional branches mispredict penalties: $mispred_pen_cond_br_sum" | cat >> $out
echo "Average mispredict rate                                  : $accuracy %" | cat >> $out
echo "Average penalty per misprediction                        : $avg_pen_per_miss" | cat >> $out