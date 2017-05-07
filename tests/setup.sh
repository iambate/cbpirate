#!/bin/bash
set -e

THIS_FILE=`readlink -f $0`
THIS_DIR=`dirname $THIS_FILE`
mkdir -p $THIS_DIR/outputs
export LD_LIBRARY_PATH=/usr/local/lib
g++ -fcilkplus $THIS_DIR/mat_mul_ikj_ipara.c -D MATRIX_SIZE=2048 -o $THIS_DIR/mat_mul_ikj_ipara
g++ -fcilkplus $THIS_DIR/mat_mul_para_rec.c -D MATRIX_SIZE=2048 -D M=128 -o $THIS_DIR/mat_mul_para_rec

taskset -c 0,1 $THIS_DIR/mat_mul_ikj_ipara > $THIS_DIR/outputs/mat_mul_ikj_ipara.wocbpirate.out
$THIS_DIR/../runcbpirate.sh $THIS_DIR/mat_mul_ikj_ipara > $THIS_DIR/outputs/mat_mul_ikj_ipara.withcbpirate.out

taskset -c 0,1 $THIS_DIR/mat_mul_para_rec > $THIS_DIR/outputs/mat_mul_para_rec.wocbpirate.out
$THIS_DIR/../runcbpirate.sh $THIS_DIR/mat_mul_para_rec > $THIS_DIR/outputs/mat_mul_para_rec.withcbpirate.out
