#include <iostream>
#include <stdlib.h>
#include<cilk/cilk.h>
#include <fstream>

#define MATRIX_SIZE 16

using namespace std;

int dist [MATRIX_SIZE][MATRIX_SIZE];
int next [MATRIX_SIZE][MATRIX_SIZE];

void FloydWarshall(int (&dist)[MATRIX_SIZE][MATRIX_SIZE])
{
   cilk_for (int k = 0; k < MATRIX_SIZE; k++) // standard Floyd-Warshall implementation
	cilk_for (int i = 0; i < MATRIX_SIZE; i++)
		cilk_for (int j = 0; j < MATRIX_SIZE; j++)
			dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
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
FloydWarshall(dist);

for (int i = 0; i < MATRIX_SIZE; i++)	{
	for (int j = 0; j < MATRIX_SIZE; j++)	{
		cout << dist[i][j] << " ";
	}
	cout << endl;
}

return 0;

}
