#include <iostream>
#include <stdlib.h>
#include<cilk/cilk.h>
#include <fstream>
#include <string>
#include <vector>
#include<sys/time.h>

using namespace std;

/*
 * function to return the substition cost to make the two strings same as
 * each other
 */
int S(char xi, char yi)
{
	if (xi == yi)
		return 0;
	else
		return 2;
}

/*
 * finds the gap between two given strings
 */
void LoopGap(vector<vector<int> > (&dist), string x, string y, int m, int n)
{
	for (int t = 2; t <= m + n; t++)	{
		cilk_for (int i = max(1, t - n); i < min(t - 1, m) ; i++)	{
			int j = t - i;
			//cout << i << " " << j << endl;
			dist[i][j] = dist[i - 1][j - 1] + S(x[i], y[i]);
			for (int k = 0; k < j; k++)
				dist[i][j] = min(dist[i][j], dist[i][k]) + 1;
			for (int k = 0; k < i; k++)
				dist[i][j] = min(dist[i][j], dist[k][j]) + 1;
		}
	}
	
}

int main()
{
string x;
string y;
ifstream infile("./input_gap.txt");

infile >> x >> y;

int n = x.size();
int m = y.size(); 


vector<vector<int> > dist;
for (int i = 0; i <= n; i++)	{
	vector<int> v;
	dist.push_back(v);
}

for (int i = 0; i <= m; i++)	{
	for (int j = 0; j <= n; j++)	{
		dist[i].push_back(0);
	}
}

/*
for (int i = 0; i <= m; i++)	{
	for (int j = 0; j <= n; j++)	{
		cout << dist[i][j] << " ";
	}
	cout << endl;
}
*/
struct timeval start_time,end_time;
gettimeofday(&start_time,NULL);
LoopGap(dist, x, y, m, n);
gettimeofday(&end_time, NULL);
cout << (end_time.tv_sec+(double)end_time.tv_usec/1000000) - (start_time.tv_sec+(double)start_time.tv_usec/1000000)  << endl;

cout << dist[m-1][n-1] + 1 << " is the distance between the two strings" << endl;

/*
for (int i = 0; i <= m; i++)	{
	for (int j = 0; j <= n; j++)	{
		cout << dist[i][j] << " ";
	}
	cout << endl;
}
*/

return 0;

}
