#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<limits.h>
#include<algorithm>
#include<cilk/cilk.h>
#include<sys/time.h>
using namespace std;

#define INFINITY 99999

int print_matrix(int **d)
{
	for (int i = 0; i < MATRIX_SIZE; i++)	{
		for (int j = 0; j < MATRIX_SIZE; j++)	{
			cout << d[i][j] << "\t";
		}
		cout << endl;
	}
	return 0;	
}
int check_matrix(int **c, int **d)
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

int serial(int **d)
{
        for (int i = 0; i < MATRIX_SIZE; i++)
                for (int j = i + 2; j < MATRIX_SIZE; j++)
                        for(int k = i; k <= j; k++)
                                d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
	
	return 0;
}

int base_matrix_operation(int **c, int start_i , int start_j, int n)
{
	for (int i = start_i; i < start_i + n-1; i++)
		for (int j = i + 2; j < n; j++)
			for(int k = i; k <= j; k++)
				c[i][j] = min(c[i][j], c[i][k] + c[k][j]);
	return 0;
}

int C(int **c, int i, int j, int u_i, int u_j, int v_i, int v_j, int n)
{
	if (n <= BASE_SIZE)	{
		for (int a = 0; a < n; a++)	{
			for (int b = 0; b < n; b++)	{
				for (int e = 0; e < n; e++)	{
					c[i + a][j + b] = min(c[i + a][j + b], c[u_i+a][u_j+e] + c[v_i+e][v_j+b]);
				}
			}
		}
	}
	else	{
		cilk_spawn C(c, i, j, u_i, u_j, v_i, v_j, n/2);
		cilk_spawn C(c, i, j+n/2, u_i, u_j, v_i, v_j+n/2, n/2);
		cilk_spawn C(c, i+n/2, j, u_i+n/2, u_j, v_i, v_j, n/2);
		C(c, i+n/2, j+n/2, u_i+n/2, u_j, v_i, v_j+n/2, n/2);
		cilk_sync;

		cilk_spawn C(c, i, j, u_i, u_j + n/2, v_i+n/2, v_j, n/2);
		cilk_spawn C(c, i, j+n/2, u_i, u_j+n/2, v_i+n/2, v_j, n/2);
		cilk_spawn C(c, i+n/2, j, u_i+n/2, u_j+n/2, v_i+n/2, v_j, n/2 );
		C(c, i+n/2, j+n/2, u_i+n/2, u_j+n/2, v_i+n/2, v_j+n/2, n/2);
		cilk_sync;
	}
	return 0;
}

int B(int **c, int i, int j, int u_i, int u_j, int v_i, int v_j, int n)
{
	if (n <= BASE_SIZE)	{
		//base_matrix_operation(c, i, j, n);

		for (int a = 0; a < n; a++) {
			int b = 0;
			int e = n - 1;
			c[i+a][j+b] = min(c[i+a][j+b], c[u_i+a][u_j+e] + c[i+n-1][j+b]);
		}
		for (int a = 0; a < n; a++)	{
			for (int b = 1; b < n; b++)	{
				//int b = a - t;
				for (int e = 0; e < n; e++)	{
					c[i+a][j+b] = min(c[i+a][j+b], c[u_i+a][u_j+e] + c[i+e][j+b]);
				}
				
				for (int e = 0; e < b; e++)	{
					c[i+a][j+b] = min(c[i+a][j+b], c[i + a][j + e] + c[v_i + e][v_j + b]);
				}
			}
		}
		
	}
	else	{
		B(c, i+n/2, j, u_i+n/2, u_j+n/2, v_i, v_j, n/2);
		cilk_spawn C(c, i, j, u_i, u_j+n/2, i+n/2,j, n/2);
		C(c, i+n/2, j+n/2, i+n/2, j, v_i, v_j + n/2, n/2);
		cilk_sync;
		
		cilk_spawn B(c, i, j, u_i, u_j, v_i, v_j, n/2);
		B(c, i+n/2, j+n/2, u_i+n/2, u_j+n/2, v_i+n/2, v_j+n/2, n/2);
		cilk_sync;

		C(c, i, j+n/2, u_i, u_j+n/2, i+n/2, j+n/2, n/2);
		C(c, i, j+n/2, i, j, v_i, v_j+n/2, n/2);
		B(c, i, j+n/2, u_i, u_j, v_i+n/2, v_j+n/2, n/2);
	}
	return 0;
}

// dividing function
int A(int **c, int i, int j, int n)
{
	if (n <= BASE_SIZE)	{
		for (int t = 2; t < n; t++)	{
			for (int a = 0; a < n-t; a++)	{
				int b = t+a;
				for (int e = a; e <= b; e++)	{
					c[i+a][j+b] = min(c[i+a][j+b], c[i+a][j+e] + c[i+e][j+b]);
				}
			}
		}
	}
	else	{
		cilk_spawn A(c, i, j, n/2);
		A(c, i + n/2, j + n/2, n/2);
		cilk_sync;
		
		B(c, i, j + n/2, i, j, i + n/2, j + n/2, n/2);
	}
	return 0;
}

int main()
{
	int *c[MATRIX_SIZE];
	int *d[MATRIX_SIZE];
	
    for(int i=0; i< MATRIX_SIZE; i++) {
        c[i] = (int *)malloc(sizeof(int)*MATRIX_SIZE);
        d[i] = (int *)malloc(sizeof(int)*MATRIX_SIZE);
    }
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
	A(c, 0, 0, MATRIX_SIZE);
	/*serial(d);
	if (check_matrix(c, d))
		cout << "correct" << endl;
	else
		cout << "wrong" << endl;
        */
	gettimeofday(&end_time, NULL);
        cout << (end_time.tv_sec+(double)end_time.tv_usec/1000000) - (start_time.tv_sec+(double)start_time.tv_usec/1000000)  << endl;
	
	//print_matrix(c);
	//print_matrix(d);
    for(int i=0; i< MATRIX_SIZE; i++) {
        free(c[i]);
        free(d[i]);
    }
	return 0;
}
