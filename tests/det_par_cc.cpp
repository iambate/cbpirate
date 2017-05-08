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

void par_prefix_sum(vector<int>& S, vector<int>& newS)
{
	if (S.size() == 1)	{
		newS[0] = S[0];
		return;
	}

	vector <int> y(S.size() / 2);
	vector <int> z(S.size() / 2);

	cilk_for (int i = 0; i < (S.size() / 2); i++) {
		y[i] = S[2 * i] + S[2 * i + 1];
#ifdef DEBUG
	cout << "par_pref_sum ended: " << i << " s.size(): " << S.size() << endl;
#endif
	}

	par_prefix_sum(y, z);
	vector<int>().swap(y);
	cilk_for (int i = 0; i < S.size(); i++)	{
		if (i == 0)
			newS[i] = S[i];
		else if (i % 2 != 0)
			newS[i] = z[i/2];
		else
			newS[i] = z[(i-1) / 2] + S[i];
#ifdef DEBUG
	cout << "par_pref_sum ended: " << i << " s.size(): " << S.size() << endl;
#endif
	}
	vector<int>().swap(z);

}

int par_sum(vector <int> &S, vector<int> &newS, int n)
{
	int sum = 0;

	if (n == 1)
		sum = S[0];
	else {
		cilk_for(int i = 0; i < (n / 2); i++) {
			newS[i] = S[2 * i] + S[2 * i + 1];
		}

		if(n % 2 != 0)
			newS[(n / 2)-1] += S[n - 1];
		sum = par_sum(S, newS, (n / 2));
	}
	
#ifdef DEBUG
	cout << "par_sum ended: " << n << endl;
#endif
	return sum;


}

void find_roots(int n, vector<int> &S)
{
	int flag = true;
	
	while (flag)	{
		flag = false;
		cilk_for (int i = 0; i < n; i++)	{
			S[i] = S[S[i]];
			if (S[i] != S[S[i]])
				flag = true;
		}
	}
}

void par_deterministic_cc(int n, vector<pair<int,int > >& E, vector<VERTEX_LABEL>& L)
{
	int esize = E.size();
	if (esize == 0)
		return;

	vector <int> l2h(n), h2l(n), S(esize);

	cilk_for(int i = 0; i < n; i++)	{
		l2h[i] = h2l[i] = 0;
	}

	cilk_for (int i = 0; i < esize; i++)	{
		int u = E[i].first;
		int v = E[i].second;
		if (u < v)
			l2h[u] = 1;
		else
			h2l[u] = 1;
	}
	
	vector<int> new_sum1(n/2);
	int n1 = par_sum(l2h, new_sum1, n);
	vector<int>().swap(l2h);
	vector<int>().swap(new_sum1);
	
	vector<int> new_sum2(n/2);
	int n2 = par_sum(h2l, new_sum2, n);
	vector<int>().swap(h2l);
	vector<int>().swap(new_sum2);

	cilk_for (int i = 0; i < esize; i++)	{
		int u = E[i].first;
		int v = E[i].second;	
	
		if (n1 >= n2 && u < v)
			L[u] = v;
		else if (n1 < n2 && u > v)
			L[u] = v;
			
	}

	find_roots(n, L);
	cilk_for (int i = 0; i < esize; i++)	{
		int u = E[i].first;
		int v = E[i].second;	
	
		if (L[u] != L[v])
			S[i] = 1;
		else
			S[i] = 0;

	}

	vector <int> newS(S.size());
	par_prefix_sum(S, newS);
	vector<int>().swap(S);

	vector<pair<int, int> > F(newS[esize - 1]);

	cilk_for (int i = 0; i < esize; i++)	{
		int u = E[i].first;
		int v = E[i].second;	
	
		if (L[u] != L[v])
			F[newS[i]-1] = make_pair(L[u], L[v]);	
	}

	vector<pair<int,int > >().swap(E);
	par_deterministic_cc (n, F, L);
	vector<pair<int,int > >().swap(F);
}

int main(int argc, char ** argv)
{

	ifstream infile(argv[1]);
	ofstream outfile;
	outfile.open(argv[2], ofstream::out | ofstream::trunc);

	int NUM_VERTICES, NUM_EDGES;
	infile >> NUM_VERTICES;
	infile >> NUM_EDGES;
	
	srand(time(NULL));
	vector <VERTEX_LABEL> L(NUM_VERTICES);
	
	cilk_for(int i = 0; i < NUM_VERTICES; i++)
		L[i] = i;

	vector <pair <int, int> > E;	
	for (int i = 0; i < NUM_EDGES; i++)	{
		int u, v;
		infile >> u;
		infile >> v;

		pair <int, int> np = make_pair(u-1, v-1);
		pair <int, int> np2 = make_pair(v-1, u-1);
		E.push_back(np);
		E.push_back(np2);
	}
	NUM_EDGES *= 2;

	infile.close();

	struct timeval start_time,end_time;
	gettimeofday(&start_time,NULL);	
	par_deterministic_cc(L.size(), E, L);
	gettimeofday(&end_time,NULL);
        cout << (end_time.tv_sec+(double)end_time.tv_usec/1000000) - (start_time.tv_sec+(double)start_time.tv_usec/1000000)  << endl;

#ifdef DEBUG
	for (int i=0; i < L.size(); i++){
		cout << "L[i]  i: " << i << "L[i]: " << L[i] << endl;
	}
#endif
	sort(L.begin(), L.end());

	vector<int> count;
	for (int i = 0; i < L.size()-1; i++){
		if(L[i] != L[i+1]) {
			count.push_back(i+1);
		}
	}
	count.push_back(L.size());
	vector<VERTEX_LABEL>().swap(L);
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

	outfile.close();

	return 0;
}
