#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<limits.h>
#include<algorithm>
#include<cilk/cilk.h>
#include<time.h>
#include<sys/time.h>
using namespace std;

void fill_matrix(int **mat){
	for(int i=0; i < MATRIX_SIZE; i++){
		for(int j=0; j < MATRIX_SIZE;j++){
			mat[i][j] = rand() % 1000;
		}
	}
}
void fill_matrix_zero(int **mat){
	for(int i=0; i < MATRIX_SIZE; i++){
		for(int j=0; j < MATRIX_SIZE;j++){
			mat[i][j] = 0;
		}
	}
}
void print_matrix(int **mat){
	for(int i=0; i < MATRIX_SIZE; i++){
		for(int j=0; j < MATRIX_SIZE;j++){
			cout << mat[i][j] << " ";
		}
		cout << endl;
	}
}

void multiply_matrix(int **mat1, int **mat2, int **result){
	cilk_for(int i=0;i<MATRIX_SIZE;i++){
		for(int k=0;k<MATRIX_SIZE;k++){
			for(int j=0;j<MATRIX_SIZE;j++){
				result[i][j] = result[i][j] + mat1[i][k]*mat2[k][j];
			}
		}
	}	
}
int main(){
	int *mat1[MATRIX_SIZE], *mat2[MATRIX_SIZE], *result[MATRIX_SIZE];
    for(int i=0; i < MATRIX_SIZE; i++) {
        mat1[i] = (int *)malloc(sizeof(int)*MATRIX_SIZE);
        mat2[i] = (int *)malloc(sizeof(int)*MATRIX_SIZE);
        result[i] = (int *)malloc(sizeof(int)*MATRIX_SIZE);
    }
    struct timeval start_time,end_time;
	fill_matrix(mat1);
	fill_matrix(mat2);
	fill_matrix_zero(result);
        gettimeofday(&start_time,NULL);
	multiply_matrix(mat1, mat2, result);
        gettimeofday(&end_time,NULL);
        cout << (end_time.tv_sec+(double)end_time.tv_usec/1000000) - (start_time.tv_sec+(double)start_time.tv_usec/1000000)  << endl;
    for(int i=0; i < MATRIX_SIZE; i++) {
        free(mat1[i]);
        free(mat2[i]);
        free(result[i]);
    }
	return 0;
}
