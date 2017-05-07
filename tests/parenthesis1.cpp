#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<limits.h>
#include<algorithm>
#include<cilk/cilk.h>
#include<sys/time.h>
using namespace std;

#define INFINITY 99999
#define MATRIX_SIZE 16
#define BASE_SIZE 4

int print_matrix(int (&d)[MATRIX_SIZE][MATRIX_SIZE])
{
	for (int i = 0; i < MATRIX_SIZE; i++)	{
		for (int j = 0; j < MATRIX_SIZE; j++)	{
			cout << d[i][j] << "\t";
		}
		cout << endl;
	}
	return 0;	
}
int check_matrix(int (&c)[MATRIX_SIZE][MATRIX_SIZE], int (&d)[MATRIX_SIZE][MATRIX_SIZE])
{
	int op = 1;
	for (int i = 0; i < MATRIX_SIZE; i++)	{
		for (int j = 0; j < MATRIX_SIZE; j++)	{
			if (c[i][j] != d[i][j])
				op = 0;
		}
	}

	return op;
}

int serial(int (&d)[MATRIX_SIZE][MATRIX_SIZE])
{
        for (int i = 0; i < MATRIX_SIZE; i++)
                for (int j = i + 2; j < MATRIX_SIZE; j++)
                        for(int k = i; k <= j; k++)
                                d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
	
	return 0;
}

int main()
{
	int c[MATRIX_SIZE][MATRIX_SIZE];
	int d[MATRIX_SIZE][MATRIX_SIZE];
	
	// initializing matrix
	for (int i = 0; i < MATRIX_SIZE; i++)	{
		for (int j = 0; j < MATRIX_SIZE; j++)	{
			if (j == i + 1)	{
				c[i][j] = j;
				d[i][j] = j;
			} else	{
				c[i][j] = (unsigned int)INFINITY;
				d[i][j] = (unsigned int)INFINITY;
					
			}
		}
	}
	
	// calling A(mat, n)
	struct timeval start_time,end_time;
        gettimeofday(&start_time,NULL);
	
	serial(d);
	/*
	if (check_matrix(c, d))
		cout << "correct" << endl;
	else
		cout << "wrong" << endl;
        */
	gettimeofday(&end_time, NULL);
        cout << (end_time.tv_sec+(double)end_time.tv_usec/1000000) - (start_time.tv_sec+(double)start_time.tv_usec/1000000)  << endl;
	
	//print_matrix(c);
	//print_matrix(d);
	return 0;
}
