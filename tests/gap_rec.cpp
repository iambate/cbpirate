#include <iostream>
#include <stdlib.h>
#include<cilk/cilk.h>
#include <fstream>
#include <string>
#include <vector>

#define MATRIX_SIZE 8
#define BASE_SIZE 2

using namespace std;
vector<vector<int> > dist;

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
void LoopGap(string x, string y, int n, int xx, int xy)
{
	for (int i = 1; i <= n; i++)	{
		for (int j = 1; j <= n; j++)	{
			dist[xx + i][xy + j] = dist[xx + i - 1][xy + j - 1] + 
						S(x[xx + i - 1], y[xy + j - 1]);
		}
	}
	for (int k = 1; k <= n; k++)	{
		for (int i = 1; i <= n; i++)	{
			for (int j = 1; j <= n; j++)	{
				//cout << "here AGap " <<  xx + i << " " << xy + j  << " " << xx + k << endl;
				dist[xx + i][xy + j] = min(dist[xx + i][xy + j], 
								dist[xx + k][xy + j]) + 1;
			}
		}
	}
}

void CGap(string &x, string &y, int n, int xx, int xy, int vx, int vy)
{
	if (n <= BASE_SIZE)	{
		for (int i = 1; i <= n; i++)	{
			for (int j = 1; j <= n; j++)	{
				dist[xx + i][xy + j] = dist[xx + i - 1][xy + j - 1] + S(x[xx + i - 1], y[xy + j- 1]);
			}
		}
		for (int k = 1; k <= n; k++)	{
			for (int i = 1; i <= n; i++)	{
				for (int j = 1; j <= n; j++)	{
					//cout << "here CGap " <<  xx + i << " " << xy + j  << " " << vx + k << endl;
					dist[xx + i][xy + j] = min(dist[xx + i][xy + j], dist[vx + k][vy + j]) + 1;
				}
			}
		}
	
	}
	else	{
		cilk_spawn CGap( x, y, n/2, xx, xy, vx, vy);
		cilk_spawn CGap( x, y, n/2, xx, xy + n/2, vx, vy + n/2);	
		cilk_spawn CGap( x, y, n/2, xx + n/2, xy, vx, vy);
		CGap( x, y, n/2, xx + n/2, xy + n/2, vx, vy + n/2);
		cilk_sync;

		cilk_spawn CGap(x, y, n/2, xx, xy, vx + n/2, vy);
		cilk_spawn CGap(x, y, n/2, xx, xy + n/2, vx + n/2, vy + n/2);
		cilk_spawn CGap(x, y, n/2, xx + n/2, xy, vx + n/2, vy);
		CGap(x, y, n/2, xx + n/2, xy + n/2, vx + n/2, vy + n/2);
		cilk_sync;
	}
}

void BGap(string &x, string &y, int n, int xx, int xy, int ux, int uy)
{
	if (n <= BASE_SIZE)	{
		for (int i = 1; i <= n; i++)	{
			for (int j = 1; j <= n; j++)	{
				dist[xx + i][xy + j] = dist[xx + i - 1][xy + j - 1] + S(x[xx + i - 1], y[xy + j - 1]);
			}
		}
		for (int k = 1; k <= n; k++)	{
			for (int i = 1; i <= n; i++)	{
				for (int j = 1; j <= n; j++)	{
					//cout << "here BGap " <<  xx + i << " " << xy + j  << " " << uy + k << endl;
					dist[xx + i][xy + j] = min(dist[xx + i][xy + j], dist[ux + i][uy + k]) + 1;
				}
			}
		}
	
	}
	else	{
		cilk_spawn BGap(x, y, n/2, xx, xy, ux, uy);
		cilk_spawn BGap(x, y, n/2, xx, xy + n/2, ux, uy);
		cilk_spawn BGap(x, y, n/2, xx + n/2, xy, ux + n/2, uy);
		BGap(x, y, n/2, xx + n/2, xy + n/2, ux + n/2, uy);
		cilk_sync;
		
		cilk_spawn BGap(x, y, n/2, xx, xy, ux, uy + n/2);
		cilk_spawn BGap(x, y, n/2, xx, xy + n/2, ux, uy + n/2);
		cilk_spawn BGap(x, y, n/2, xx + n/2, xy, ux + n/2, uy + n/2);
		BGap(x, y, n/2, xx + n/2, xy + n/2, ux + n/2, uy + n/2);
		cilk_sync;
	}
}

void AGap(string &x, string &y, int n, int xx, int xy)
{
	if (n <= BASE_SIZE)	{
		LoopGap(x, y, n, xx, xy);
	}
	else	{
		AGap(x, y, n/2, xx, xy);
		
		cilk_spawn BGap(x, y, n/2, xx, xy + n/2, xx, xy);
		CGap(x, y, n/2, xx + n/2, xy, xx, xy);
		cilk_sync;
		
		cilk_spawn AGap(x, y, n/2, xx, xy + n/2);
		AGap(x, y, n/2, xx + n/2, xy);
		cilk_sync;
		
		BGap(x, y, n/2, xx + n/2, xy + n/2, xx + n/2, xy);
		CGap(x, y, n/2, xx + n/2, xy + n/2, xx, xy + n/2);
		AGap(x, y, n/2, xx + n/2, xy + n/2);
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
AGap(x, y, n, 1, 1);

cout << dist[m-1][n-1] << " is the distance between the two strings" << endl;


for (int i = 0; i <= m; i++)	{
	for (int j = 0; j <= n; j++)	{
		cout << dist[i][j] << " ";
	}
	cout << endl;
}


return 0;

}
