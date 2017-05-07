#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<limits.h>
#include<algorithm>
#include<cilk/cilk.h>
#include<time.h>
#include <sys/time.h>
using namespace std;

struct matrix{
	int *mat[MATRIX_SIZE];
	int start_row;
	int start_col;
	int end_row;
	int end_col;
};

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
		cout << endl;	}
}
void multiply_matrix(struct matrix Z, struct matrix X, struct matrix Y){
	if(Z.end_row - Z.start_row <= M){
		int m = Z.end_row - Z.start_row;
		for(int k=0; k<m; k++){
			for(int i=0;i<m;i++){
				for (int j=0;j<m;j++){
					Z.mat[Z.start_row+i][Z.start_col+j] = Z.mat[Z.start_row+i][Z.start_col+j] + X.mat[X.start_row+i][X.start_col+k]*Y.mat[Y.start_row+k][Y.start_col+j];
				}
			}
		}
		Z.mat[Z.start_row][Z.start_col] = Z.mat[Z.start_row][Z.start_col] + X.mat[X.start_row][X.start_col] * Y.mat[Y.start_row][Y.start_col];
	} else {
		struct matrix Z11 = Z, Z12 = Z, Z21 = Z, Z22 = Z;
		struct matrix X11 = X, X12 = X, X21 = X, X22 = X;
		struct matrix Y11 = Y, Y12 = Y, Y21 = Y, Y22 = Y;
		// Z11
		Z11.end_row = Z.start_row + (Z.end_row - Z.start_row)/2;
		Z11.end_col = Z.start_col + (Z.end_col - Z.start_col)/2;
		// Z12
		Z12.start_col = Z.start_col + (Z.end_col - Z.start_col)/2;
		Z12.end_row = Z.start_row + (Z.end_row - Z.start_row)/2;
		//Z21
		Z21.start_row = Z.start_row + (Z.end_row - Z.start_row)/2;
		Z21.end_col = Z.start_col + (Z.end_col - Z.start_col)/2;
		//Z22
		Z22.start_row = Z.start_row + (Z.end_row - Z.start_row)/2;
		Z22.start_col = Z.start_col + (Z.end_col - Z.start_col)/2;

		// X11
		X11.end_row = X.start_row + (X.end_row - X.start_row)/2;
		X11.end_col = X.start_col + (X.end_col - X.start_col)/2;
		// X12
		X12.start_col = X.start_col + (X.end_col - X.start_col)/2;
		X12.end_row = X.start_row + (X.end_row - X.start_row)/2;
		//X21
		X21.start_row = X.start_row + (X.end_row - X.start_row)/2;
		X21.end_col = X.start_col + (X.end_col - X.start_col)/2;
		//X22
		X22.start_row = X.start_row + (X.end_row - X.start_row)/2;
		X22.start_col = X.start_col + (X.end_col - X.start_col)/2;

		// Y11
		Y11.end_row = Y.start_row + (Y.end_row - Y.start_row)/2;
		Y11.end_col = Y.start_col + (Y.end_col - Y.start_col)/2;
		// Y12
		Y12.start_col = Y.start_col + (Y.end_col - Y.start_col)/2;
		Y12.end_row = Y.start_row + (Y.end_row - Y.start_row)/2;
		//Y21
		Y21.start_row = Y.start_row + (Y.end_row - Y.start_row)/2;
		Y21.end_col = Y.start_col + (Y.end_col - Y.start_col)/2;
		//Y22
		Y22.start_row = Y.start_row + (Y.end_row - Y.start_row)/2;
		Y22.start_col = Y.start_col + (Y.end_col - Y.start_col)/2;

		//Main logic
		cilk_spawn multiply_matrix(Z11, X11, Y11);
		cilk_spawn multiply_matrix(Z12, X11, Y12);
		cilk_spawn multiply_matrix(Z21, X21, Y11);
		           multiply_matrix(Z21, X21, Y12);
		cilk_sync;

		cilk_spawn multiply_matrix(Z11, X12, Y21);
		cilk_spawn multiply_matrix(Z12, X12, Y22);
		cilk_spawn multiply_matrix(Z21, X22, Y21);
		           multiply_matrix(Z22, X22, Y22);
		cilk_sync;
		/*
		cout << "calling Z11" << endl; 
		multiply_matrix(Z11, X11, Y11);
		cout << "calling Z12" << endl; 
		multiply_matrix(Z12, X11, Y12);
		cout << "calling Z21" << endl; 
		multiply_matrix(Z21, X21, Y11);
		cout << "calling Z21" << endl; 
		multiply_matrix(Z21, X21, Y12);
		cout << "calling Z11" << endl; 
		multiply_matrix(Z11, X12, Y21);
		cout << "calling Z12" << endl; 
		multiply_matrix(Z12, X12, Y22);
		cout << "calling Z21" << endl; 
		multiply_matrix(Z21, X22, Y21);
		cout << "calling Z22" << endl; 
		multiply_matrix(Z22, X22, Y22);
		*/
	}
}
int main(){
	int *mat1[MATRIX_SIZE], *mat2[MATRIX_SIZE], *result[MATRIX_SIZE];
    for(int i=0;i<MATRIX_SIZE;i++) {
        mat1[i] = (int *)malloc(sizeof(int)*MATRIX_SIZE);
        mat2[i] = (int *)malloc(sizeof(int)*MATRIX_SIZE);
        result[i] = (int *)malloc(sizeof(int)*MATRIX_SIZE);
    }

    struct timeval start_time,end_time;
	struct matrix X = {mat1, 0, 0, MATRIX_SIZE, MATRIX_SIZE};
	struct matrix Y = {mat2, 0, 0, MATRIX_SIZE, MATRIX_SIZE};
	struct matrix Z = {result, 0, 0, MATRIX_SIZE, MATRIX_SIZE};
	fill_matrix(mat1);
	fill_matrix(mat2);
	fill_matrix_zero(result);

/*
	X.mat = mat1;
	X.start_row = 0;
	X.start_col = 0;
	X.end_row = MATRIX_SIZE;
	X.end_col = MATRIX_SIZE;

	Y.mat = mat2;
	Y.start_row = 0;
	Y.start_col = 0;
	Y.end_row = MATRIX_SIZE;
	Y.end_col = MATRIX_SIZE;

	Z.mat = result;
	Z.start_row = 0;
	Z.start_col = 0;
	Z.end_row = MATRIX_SIZE;
	Z.end_col = MATRIX_SIZE;
*/
	gettimeofday(&start_time,NULL);
	multiply_matrix(Z, X, Y);
	gettimeofday(&end_time,NULL);
	cout << (end_time.tv_sec+(double)end_time.tv_usec/1000000) - (start_time.tv_sec+(double)start_time.tv_usec/1000000)  << endl;
    for(int i=0;i<MATRIX_SIZE;i++) {
        free(mat1[i]);
        free(mat2[i]);
        free(result[i]);
    }
	return 0;
}
