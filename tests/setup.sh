#!/bin/bash
set -e

THIS_FILE=`readlink -f $0`
THIS_DIR=`dirname $THIS_FILE`
mkdir -p $THIS_DIR/outputs

export CB_FILE=$THIS_DIR/../mm_bw2.txt
export LD_LIBRARY_PATH=/usr/local/lib
g++ -fcilkplus $THIS_DIR/mat_mul_ikj_ipara.c -D MATRIX_SIZE=2048 -o $THIS_DIR/mat_mul_ikj_ipara
g++ -fcilkplus $THIS_DIR/mat_mul_para_rec.c -D MATRIX_SIZE=2048 -D M=128 -o $THIS_DIR/mat_mul_para_rec

g++ -fcilkplus $THIS_DIR/parenthesis_iter.cpp -D MATRIX_SIZE=2048 -o $THIS_DIR/parenthesis_iter
g++ -fcilkplus $THIS_DIR/parenthesis_rec.cpp -D MATRIX_SIZE=2048 -D BASE_SIZE=8 -o $THIS_DIR/parenthesis_rec

#g++ -fcilkplus $THIS_DIR/gap_iter.cpp -D MATRIX_SIZE=2048 -o $THIS_DIR/gap_iter
#g++ -fcilkplus $THIS_DIR/gap_rec.cpp -D MATRIX_SIZE=2048 -D BASE_SIZE=8 -o $THIS_DIR/gap_rec

#g++ -O0 -fcilkplus $THIS_DIR/gap_iter.cpp -o $THIS_DIR/gap_iter
g++ -O0 -fcilkplus $THIS_DIR/gap_rec.cpp -o $THIS_DIR/gap_rec

#g++ -O0 -fcilkplus $THIS_DIR/fwapsp_iter.cpp -o $THIS_DIR/fwapsp_iter
#g++ -O0 -fcilkplus $THIS_DIR/fwapsp_rec.cpp -o $THIS_DIR/fwapsp_rec

#taskset -c 0,1 $THIS_DIR/mat_mul_ikj_ipara > $THIS_DIR/outputs/mat_mul_ikj_ipara.wocbpirate.out
#$THIS_DIR/../runcbpirate.sh $THIS_DIR/mat_mul_ikj_ipara > $THIS_DIR/outputs/mat_mul_ikj_ipara.withcbpirate.out
#taskset -c 0,1 $THIS_DIR/mat_mul_para_rec > $THIS_DIR/outputs/mat_mul_para_rec.wocbpirate.out
#$THIS_DIR/../runcbpirate.sh $THIS_DIR/mat_mul_para_rec > $THIS_DIR/outputs/mat_mul_para_rec.withcbpirate.out

#taskset -c 0,1 $THIS_DIR/parenthesis_iter > $THIS_DIR/outputs/parenthesis_iter.wocbpirate.out
#$THIS_DIR/../runcbpirate.sh $THIS_DIR/parenthesis_iter > $THIS_DIR/outputs/parenthesis_iter.withcbpirate.out
#taskset -c 0,1 $THIS_DIR/parenthesis_rec > $THIS_DIR/outputs/parenthesis_rec.wocbpirate.out
#$THIS_DIR/../runcbpirate.sh $THIS_DIR/parenthesis_rec > $THIS_DIR/outputs/parenthesis_rec.withcbpirate.out
