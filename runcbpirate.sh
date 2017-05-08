#!/bin/bash

set -e
THIS_FILE=`readlink -f $0`
THIS_DIR=`dirname $THIS_FILE`
# Disable prefetching
# link: https://software.intel.com/en-us/articles/disclosure-of-hw-prefetcher-control-on-some-intel-processors
#sudo ../msr-tools-1.1.2/wrmsr -p 0 0x1a4 0x7

if [ -z "$CB_FILE" ]; then
    echo "Please specify CB_FILE"
    exit
fi
for i in `ls -d /sys/devices/system/cpu/cpu0/cache/index*`;
do
	no_of_ways=`cat $i/ways_of_associativity`
	size=`cat $i/size`
	no_of_sets=`cat $i/number_of_sets`
    block_size=`cat $i/coherency_line_size`
done

sizekb=${size/K/}
sizenum=$((sizekb*1024))
nr_hugepages=$((no_of_ways*2 + 1))
echo $nr_hugepages > /proc/sys/vm/nr_hugepages
echo "***new hugepages values***"
grep -i hugepages /proc/meminfo
rm -f $THIS_DIR/cbpirate
gcc -O0 $THIS_DIR/cbpirate.c -o $THIS_DIR/cbpirate -lpapi -lpthread
echo $no_of_ways
echo $sizenum
echo $no_of_sets
LD_LIBRARY_PATH=/usr/local/lib taskset -c 2,3 $THIS_DIR/cbpirate -s $sizenum -w $no_of_ways -n $no_of_sets \
                                          -i 400 \
                                          -f $CB_FILE \
                                          -c 0,1 \
                                          -b $block_size -t 4 \
                                          $@

# sleep 1
# echo "***new hugepages values***"
# grep -i hugepages /proc/meminfo
# ps -eF|grep cbpirate
