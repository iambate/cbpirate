#include <iostream>
#include <stdlib.h>
#include<cilk/cilk.h>
#include <fstream>
#include<sys/time.h>

#define MATRIX_SIZE 512
#define BASE_SIZE 4

using namespace std;

int dist [MATRIX_SIZE][MATRIX_SIZE];
int next [MATRIX_SIZE][MATRIX_SIZE];

void FloydWarshallLoop(int (&dist)[MATRIX_SIZE][MATRIX_SIZE])
{
   cilk_for (int k = 0; k < MATRIX_SIZE; k++) // standard Floyd-Warshall implementation
	cilk_for (int i = 0; i < MATRIX_SIZE; i++)
		cilk_for (int j = 0; j < MATRIX_SIZE; j++)
			dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
}

void FWD(int (&dist)[MATRIX_SIZE][MATRIX_SIZE], int n, int xx, int xy, int ux, int uy, int vx, int vy)
{
	if (n <= BASE_SIZE)	{
		for (int k = 0; k < n; k++)	{
			for (int i = 0; i < n; i++)	{
				for (int j = 0; j < n; j++)	{
					dist[xx + i][xy + j] = min(dist[xx + i][xy + j], 
								dist[ux + i][uy + k] + dist[vx + k][vy + j]);
				}
			}
		}
	}
	else	{
		cilk_spawn FWD(dist, n/2, xx, xy, ux, uy, vx, vy);
		cilk_spawn FWD(dist, n/2, xx, xy + n/2, ux, uy, vx, vy + n/2);
		cilk_spawn FWD(dist, n/2, xx + n/2, xy, ux + n/2, uy, vx, vy);
		FWD(dist, n/2, xx + n/2, xy + n/2, ux + n/2, uy, vx, vy + n/2);
		cilk_sync;
		
		cilk_spawn FWD(dist, n/2, xx, xy, ux, uy + n/2, vx + n/2, vy);
		cilk_spawn FWD(dist, n/2, xx, xy + n/2, ux, uy + n/2, vx + n/2, vy + n/2);
		cilk_spawn FWD(dist, n/2, xx + n/2, xy, ux + n/2, uy + n/2, vx + n/2, vy);
		FWD(dist, n/2, xx + n/2, xy + n/2, ux + n/2, uy + n/2, vx + n/2, vy + n/2);
		cilk_sync;
	}

}

void FWC(int (&dist)[MATRIX_SIZE][MATRIX_SIZE], int n, int xx, int xy, int vx, int vy)
{
	if (n <= BASE_SIZE)	{
		for (int k = 0; k < n; k++)	{
			for (int i = 0; i < n; i++)	{
				for (int j = 0; j < n; j++)	{
					dist[xx + i][xy + j] = min(dist[xx + i][xy + k], 
								dist[vx + i][vy + k] + dist[vx + k][vy + j]);
				}
			}
		}
	}
	else	{
		cilk_spawn FWC(dist, n/2, xx, xy, vx, vy);
		FWC(dist, n/2, xx + n/2, xy, vx, vy);
		cilk_sync;

		cilk_spawn FWD(dist, n/2, xx, xy + n/2, xx, xy, vx, vy + n/2);
		FWD(dist, n/2, xx + n/2, xy + n/2, xx + n/2, xy, vx, vy + n/2);
		
		cilk_sync;

		cilk_spawn FWC(dist, n/2, xx, xy + n/2, vx + n/2, vy + n/2);
		FWC(dist, n/2, xx + n/2, xy + n/2, vx + n/2, vy + n/2);
		cilk_sync;
		
		cilk_spawn FWD(dist, n/2, xx, xy, xx, xy + n/2, vx + n/2, vy);
		FWD(dist, n/2, xx + n/2, xy, xx + n/2, xy + n/2, vx + n/2, vy);
		cilk_sync;
	}
	
}

void FWB(int (&dist)[MATRIX_SIZE][MATRIX_SIZE], int n, int xx, int xy, int ux, int uy)
{
	if (n <= BASE_SIZE)	{
		for (int k = 0; k < n; k++)	{
			for (int i = 0; i < n; i++)	{
				for (int j = 0; j < n; j++)	{
					dist[xx + i][xy + j] = min(dist[xx + i][xy + k], 
								dist[ux + k][uy + j] + dist[xx + i][xy + k]);
				}
			}
		}
		
	}
	else	{
	
		cilk_spawn FWB(dist, n/2, xx, xy, ux, uy);
		FWB(dist, n/2, xx, xy + n/2, ux, uy);
		cilk_sync;

		cilk_spawn FWD(dist, n/2, xx + n/2, xy, ux + n/2, uy, xx, xy);
		FWD(dist, n/2, xx + n/2, xy + n/2, ux + n/2, uy, xx, xy + n/2);
		cilk_sync;

		cilk_spawn FWB(dist, n/2, xx + n/2, xy, ux + n/2, uy + n/2);
		FWB(dist, n/2, xx + n/2, xy + n/2, ux + n/2, uy + n/2);
		cilk_sync;

		cilk_spawn FWD(dist, n/2, xx, xy, ux, uy + n/2, xx + n/2, xy);
		FWD(dist, n/2, xx, xy + n/2, ux, uy + n/2, xx + n/2, xy + n/2);
		cilk_sync;
	}
}

void FWA(int (&dist)[MATRIX_SIZE][MATRIX_SIZE], int n, int xx, int xy)
{
	if (n <= BASE_SIZE)	{
		for (int k = 0; k < n; k++)	{
			for (int i = 0; i < n; i++)	{
				for (int j = 0; j < n; j++)	{
					dist[xx + i][xy + j] = min(dist[xx + i][xy + j], 
								dist[xx + i][xy + k] + dist[xx + k][xy + j]);
				}
			}
		}	
	}
	else	{
		FWA(dist, n/2, xx, xy);
		
		cilk_spawn FWB(dist, n/2, xx, xy + n/2, xx, xy);
		FWC(dist, n/2, xx + n/2, xy, xx, xy);
		cilk_sync;

		FWD(dist, n/2, xx + n/2, xy + n/2, xx + n/2, xy, xx, xy + n/2);
		FWA(dist, n/2, xx + n/2, xy + n/2);
		
		cilk_spawn FWB(dist, n/2, xx + n/2, xy, xx + n/2, xy + n/2);
		FWC(dist, n/2, xx, xy + n/2, xx + n/2, xy + n/2);
		cilk_sync;

		FWD(dist, n/2, xx, xy, xx, xy + n/2, xx + n/2, xy);
	}
}




int main()
{
// let dist be a |V| x |V| array of minimum distances initialized to infinity
	int NUM_INPUTS;

	ifstream infile("./input_fwapsp.txt");
	infile >> NUM_INPUTS;

	for (int i = 0; i <= MATRIX_SIZE; i++)	{
		for (int j = 0; j <= MATRIX_SIZE; j++)	{
			dist[i][j] = 999999;
			dist[j][i] = 999999;
			//if (i == j)
			//	dist[i][j] = 0;
		}
	}

	for (int i = 0; i < NUM_INPUTS; i++)	{
		int x, y, w;
		infile >> x >> y >> w;
		//cout << x << y << w << endl;

		dist[x][y] = w;
		dist[y][x] = w;
		//if (x == y)
		//	dist[x][y] = dist[y][x] = 0;
	}
	/*	
	for (int i = 0; i < MATRIX_SIZE; i++)	{
		for (int j = 0; j < MATRIX_SIZE; j++)	{
			cout << dist[i][j] << " ";
		}
		cout << endl;
	}
        */
	struct timeval start_time,end_time;
        gettimeofday(&start_time,NULL);
	FWA(dist, MATRIX_SIZE, 0, 0);
        gettimeofday(&end_time, NULL);
        cout << (end_time.tv_sec+(double)end_time.tv_usec/1000000) - (start_time.tv_sec+(double)start_time.tv_usec/1000000)  << endl;	

	/*
	for (int i = 0; i < MATRIX_SIZE; i++)	{
		for (int j = 0; j < MATRIX_SIZE; j++)	{
			cout << dist[i][j] << " ";
		}
		cout << endl;
	}
	*/
	return 0;

}
