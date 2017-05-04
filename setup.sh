#!/bin/bash

for i in `ls -d /sys/devices/system/cpu/cpu0/cache/index*`;
do
	no_of_ways=`cat $i/ways_of_associativity`
	size=`cat $i/size`
	no_of_sets=`cat $i/number_of_sets`
    block_size=`cat $i/coherency_line_size`
done

sizekb=${size/K/}
sizenum=$((sizekb*1024))
nr_hugepages=$((no_of_ways*2))
echo $nr_hugepages > /proc/sys/vm/nr_hugepages
echo "***new hugepages values***"
grep -i hugepages /proc/meminfo
rm -f cbpirate
gcc cbpirate.c -o cbpirate -lpapi
echo $no_of_ways
echo $sizenum
echo $no_of_sets
LD_LIBRARY_PATH=/usr/local/lib taskset -c 3 ./cbpirate -s $sizenum -w $no_of_ways -n $no_of_sets \
                                          -b $block_size
