#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<cilk/cilk.h>
#include<time.h>
#include<sys/time.h>

#include<iostream>
#include<algorithm>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <utility>
#include <functional>
#include <cilk/cilk.h>

#define VERTEX_LABEL int
#define UNIQUE_ID int

#define HEAD 1
#define TAIL 0
//#define DEBUG

/*
// define the following variables at compile time
#define INFILE "myin.txt"
#define OUTFILE "myout.txt"
*/

using namespace std;

void par_prefix_sum(int *S, int *newS, int ssize)
{
	if (ssize == 1) {
		newS[0] = S[0];
		return;
	}

	int *y = (int *)malloc(sizeof(int)*(ssize / 2));
	int *z = (int *)malloc(sizeof(int)*(ssize / 2));
	cilk_for (int i = 0; i < (ssize / 2); i++)
		y[i] = S[2 * i] + S[2 * i + 1];

	par_prefix_sum(y, z, (ssize/2));
	free(y);
	cilk_for (int i = 0; i < ssize; i++)	{
		if (i == 0)
			newS[i] = S[i];
		else if (i % 2 != 0)
			newS[i] = z[i/2];
		else
			newS[i] = z[(i-1) / 2] + S[i];
	}
	free(z);

}

void par_randomized_cc(int n, int *EU, int *EV, int esize, int *L)
{
	if (esize == 0)	{
		return;
	}

	int *C = (int *)malloc(sizeof(int)*n); // stores HEAD/TAIL information for each vertex	
	cilk_for (int i = 0; i < n; i++)	{
		C[i] = rand() % 2; // chooses a random number from 0 to 1
	}
	cilk_for (int i = 0; i < esize; i++)	{
		L[EU[i]] = (C[EU[i]] == TAIL && C[EV[i]] == HEAD) ? L[EV[i]] : L[EU[i]]; 		
	}
	free(C);

	int *S = (int *)malloc(sizeof(int)*esize);
	int *newS = (int *)malloc(sizeof(int)*esize);
	cilk_for (int i = 0; i < esize; i++)	{
		S[i] = (L[EU[i]] != L[EV[i]]) ? 1 : 0;
	}
	par_prefix_sum(S, newS, esize);
	free(S);

	int fsize = newS[esize - 1];
	int *FU = (int *)malloc(fsize * sizeof(int));
	int *FV = (int *)malloc(fsize * sizeof(int));

	cilk_for (int i = 0; i < esize; i++)	{
		if (L[EU[i]] != L[EV[i]]) {
			FU[newS[i]-1] = L[EU[i]];
			FV[newS[i]-1] = L[EV[i]];
		}
	}
	free(newS);

	par_randomized_cc(n, FU, FV, fsize, L);
	free(FU);
	free(FV);
	cilk_for (int i = 0; i < esize; i++)	{
		if (EV[i] == L[EU[i]])
			L[EU[i]] = L[EV[i]];
	}
}

int main(int argc, char ** argv)
{

	ifstream infile(argv[1]);
	ofstream outfile;
	outfile.open(argv[2], ofstream::out | ofstream::trunc);

	int NUM_VERTICES, NUM_EDGES;
	infile >> NUM_VERTICES;
	infile >> NUM_EDGES;

	int *L, *EU, *EV;
	NUM_EDGES = 2*NUM_EDGES;
	L = (int *)malloc(sizeof(int)*NUM_VERTICES);
	EU = (int *)malloc(sizeof(int)*NUM_EDGES);
	EV = (int *)malloc(sizeof(int)*NUM_EDGES);
	
	for (int i = 0; i < NUM_EDGES; i+=2)	{
		int u, v;
		infile >> u;
		infile >> v;
		EU[i] = (u-1);
		EV[i] = (v-1);
		EU[i+1] = (v-1);
		EV[i+1] = (u-1);
#ifdef DEBUG
		cout << "EU[i]" << EU[i] << "EV[i]" << EV[i] <<endl;
		cout << "EU[i+1]" << EU[i+1] << "EV[i+1]" << EV[i+1] <<endl;
#endif
	}
	srand(time(NULL));
	for(int i=0;i<NUM_VERTICES;i++){
		L[i] = i;
	}
	struct timeval start_time,end_time;
	gettimeofday(&start_time,NULL);	
	par_randomized_cc(NUM_VERTICES, EU, EV, NUM_EDGES, L);
	gettimeofday(&end_time,NULL);
        cout << (end_time.tv_sec+(double)end_time.tv_usec/1000000) - (start_time.tv_sec+(double)start_time.tv_usec/1000000)  << endl;


	free(EU);
	free(EV);
#ifdef DEBUG
	for (int i=0; i < NUM_VERTICES; i++){
		cout << "L[i]  i: " << i << "L[i]: " << L[i] << endl;
	}
#endif
	sort(L, L + NUM_VERTICES);

	vector<int> count;
	for (int i = 0; i < (NUM_VERTICES - 1); i++){
		if(L[i] != L[i+1]) {
			count.push_back(i+1);
		}
	}
	count.push_back(NUM_VERTICES);
	free(L);
	sort(count.begin(), count.end());
	outfile << count.size() << endl;
	cout << count.size() << endl;
	int last_count = 0;
	vector<int> allcount;
	for(int i =0; i<count.size(); i++) {
		allcount.push_back(count[i]-last_count);
		last_count = count[i];
	}
	sort(allcount.begin(), allcount.end());
	reverse(allcount.begin(), allcount.end());
	for(int i=0; i<allcount.size(); i++) {
		outfile << allcount[i] << endl;
		//cout << allcount[i] << endl;
	}
	infile.close();
	outfile.close();
	return 0;
}
