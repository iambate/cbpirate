#!/bin/bash
set -e

THIS_FILE=`readlink -f $0`
THIS_DIR=`dirname $THIS_FILE`

rm -rf $THIS_DIR/outputs/*
mkdir -p $THIS_DIR/outputs

export LD_LIBRARY_PATH=/usr/local/lib
g++ -fcilkplus $THIS_DIR/mat_mul_ikj_ipara.c -D MATRIX_SIZE=2048 -o $THIS_DIR/mat_mul_ikj_ipara
g++ -fcilkplus $THIS_DIR/mat_mul_para_rec.c -D MATRIX_SIZE=2048 -D M=128 -o $THIS_DIR/mat_mul_para_rec

g++ -fcilkplus $THIS_DIR/parenthesis_iter.cpp -D MATRIX_SIZE=2048 -o $THIS_DIR/parenthesis_iter
g++ -fcilkplus $THIS_DIR/parenthesis_rec.cpp -D MATRIX_SIZE=2048 -D BASE_SIZE=8 -o $THIS_DIR/parenthesis_rec

# g++ -fcilkplus $THIS_DIR/gap_iter.cpp -D MATRIX_SIZE=2048 -o $THIS_DIR/gap_iter
# g++ -fcilkplus $THIS_DIR/gap_rec.cpp -D MATRIX_SIZE=2048 -D BASE_SIZE=8 -o $THIS_DIR/gap_rec

g++ -fcilkplus $THIS_DIR/gap_iter.cpp -o $THIS_DIR/gap_iter
g++ -fcilkplus $THIS_DIR/gap_rec.cpp -o $THIS_DIR/gap_rec

g++ -fcilkplus $THIS_DIR/fwapsp_iter.cpp -o $THIS_DIR/fwapsp_iter
g++ -fcilkplus $THIS_DIR/fwapsp_rec.cpp -o $THIS_DIR/fwapsp_rec

g++ -fcilkplus $THIS_DIR/rand_par_cc.cpp -o $THIS_DIR/rand_par_cc
g++ -fcilkplus $THIS_DIR/det_par_cc.cpp -o $THIS_DIR/det_par_cc

for i in {1,2};
do
    export CB_FILE=$THIS_DIR/../mm_bw$i.txt
    #taskset -c 0,1 $THIS_DIR/mat_mul_ikj_ipara > $THIS_DIR/outputs/mat_mul_ikj_ipara.wocbpirate.$i.out
    #$THIS_DIR/../runcbpirate.sh $THIS_DIR/mat_mul_ikj_ipara > $THIS_DIR/outputs/mat_mul_ikj_ipara.withcbpirate.$i.out
    #taskset -c 0,1 $THIS_DIR/mat_mul_para_rec > $THIS_DIR/outputs/mat_mul_para_rec.wocbpirate.$i.out
    #$THIS_DIR/../runcbpirate.sh $THIS_DIR/mat_mul_para_rec > $THIS_DIR/outputs/mat_mul_para_rec.withcbpirate.$i.out

    #taskset -c 0,1 $THIS_DIR/parenthesis_iter > $THIS_DIR/outputs/parenthesis_iter.wocbpirate.$i.out
    #$THIS_DIR/../runcbpirate.sh $THIS_DIR/parenthesis_iter > $THIS_DIR/outputs/parenthesis_iter.withcbpirate.$i.out
    #taskset -c 0,1 $THIS_DIR/parenthesis_rec > $THIS_DIR/outputs/parenthesis_rec.wocbpirate.$i.out
    #$THIS_DIR/../runcbpirate.sh $THIS_DIR/parenthesis_rec > $THIS_DIR/outputs/parenthesis_rec.withcbpirate.$i.out

    taskset -c 0,1 $THIS_DIR/rand_par_cc $THIS_DIR/ca-AstroPh-in.txt $THIS_DIR/ca-AstroPh-rand-out.txt > $THIS_DIR/outputs/ca-AstroPh-rand.wocbpirate.$i.out
    $THIS_DIR/../runcbpirate.sh $THIS_DIR/rand_par_cc $THIS_DIR/ca-AstroPh-in.txt $THIS_DIR/ca-AstroPh-rand-out.txt > $THIS_DIR/outputs/ca-AstroPh-rand.$i.withcbpirate.out
    taskset -c 0,1 $THIS_DIR/det_par_cc $THIS_DIR/ca-AstroPh-in.txt $THIS_DIR/ca-AstroPh-det-out.txt > $THIS_DIR/outputs/ca-AstroPh-det.wocbpirate.$i.out
    $THIS_DIR/../runcbpirate.sh $THIS_DIR/det_par_cc $THIS_DIR/ca-AstroPh-in.txt $THIS_DIR/ca-AstroPh-det-out.txt > $THIS_DIR/outputs/ca-AstroPh-det.withcbpirate.$i.out
done
