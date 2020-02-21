#include <bits/stdc++.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <ctime>
#include <time.h>
#include <vector>
#include <stack>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
using namespace std;

#define T 4
#define noOfRequests 9
#define totalRequests 600
#define maxSlotsPerReq 4
#define maxBtPerReq 5
#define slots 320
#define V 28
#define E 44

int pid;
char *dt;
int shmid;
int shmid1;
int shmid2;
int shmid3;
int shmid4;
int shmid5;
int *nor;
int *r;
int *number;
pid_t getpid(void);
int dist[V];
int Tgraph[T][T];
int TMst[T][T];
int Result[V][V];
int activePoint[V];
int graph[V][V];
int *source;
int (*req)[15];

struct f
{
	bool b[slots];
	int status;
};
f (*c)[V];

int printSolution(int parent[])
{
	for (int i = 0; i < V; i++)
	{
		if (parent[i] != -1)
		{
			Result[i][parent[i]] = graph[i][parent[i]];
			Result[parent[i]][i] = graph[parent[i]][i];
		}
	}
}

int generateRequest(int *bt, int *src, int id)
{

	for (int i = 0; i < V; i++)
		activePoint[i] = 0;
	int source = rand() % V;
	req[id][0] = source;
	*src = source;
	activePoint[source] = 1;
	int noOfDest = 3;
	for (int i = 0; i < noOfDest; i++)
	{

		int dest = rand() % V;
		while (activePoint[dest] == 1)
			dest = rand() % V;
		req[id][i + 1] = dest;
		activePoint[dest] = 1;
	}
	int noOfSlotsReq = (rand() % maxSlotsPerReq) + 1;
	int burstTime = (rand() % maxBtPerReq) + 1;

	*bt = burstTime;
	return noOfSlotsReq;
}

int minDistance(int dist[], bool sptSet[])
{
	int min = INT_MAX, min_index;
	for (int v = 0; v < V; v++)
		if (sptSet[v] == false && dist[v] <= min)
			min = dist[v], min_index = v;
	return min_index;
}

void dijkstra(int src)
{
	int parent[V];
	bool sptSet[V];
	for (int i = 0; i < V; i++)
	{
		parent[i] = -1;
		dist[i] = INT_MAX;
		sptSet[i] = false;
	}
	dist[src] = 0;
	for (int count = 0; count < V - 1; count++)
	{
		int u = minDistance(dist, sptSet);
		sptSet[u] = true;
		for (int v = 0; v < V; v++)
			if (!sptSet[v] && graph[u][v] &&
					dist[u] + graph[u][v] < dist[v])
			{
				parent[v] = u;
				dist[v] = dist[u] + graph[u][v];
			}
	}
	printSolution(parent);
}

int check(int r, int *begin) //checks for what ???
{
	int k = 0;
	int m = 0, n = 0, flag = 0;
	for (m = 0; m < V; m++)
	{
		for (n = 0; n < V; n++)
		{
			if (Result[m][n])
			{
				flag = 1;
				break;
			}
		}
		if (flag == 1)
			break;
	}
	int f = 0;
	flag = 0;
	while (f < slots && k < r)
	{
		if (c[m][n].b[f] == 0)
		{

			for (int i = m; i < V; i++)
			{
				for (int j = n + 1; j < V; j++)
				{
					if (Result[i][j])
					{
						if (c[i][j].b[f] == 0)
							continue;
						else
						{
							flag = 1;
							break;
						}
					}
				}
				if (flag == 1)
				{
					k = 0;
					f++;
					break;
				}
			}
			if (flag == 0)
			{
				k++;
				f++;
			}
			else
				flag = 0;
		}
		else
		{
			f++;
			k = 0;
		}
	}
	if (k >= r)
	{
		int g = f - r;
		*begin = g;
		for (int i = 0; i < V; i++)
		{
			for (int j = 0; j < V; j++)
			{
				if (Result[i][j])
				{
					c[i][j].status++;
					for (g = f - r; g < f; g++)
					{
						c[i][j].b[g] = 1;
					}
				}
			}
		}
		return 1;
	}
	else
		return 0;
}

void path() //prints what path ?
{
	for (int i = 0; i < V; i++)
		for (int j = i + 1; j < V; j++)
			if (Result[i][j])
				cout << i << "-->" << j << ", ";
	cout << endl;
}

void remove() //removes what ?
{
	int flag = 0;
	while (flag == 0)
	{
		flag = 1;
		for (int i = 0; i < V; i++)
		{
			if (activePoint[i] == 1)
				continue;
			int temp = 0;
			for (int j = 0; j < V; j++)
			{
				if (Result[i][j])
					temp++;
			}
			if (temp == 1)
			{
				flag = 0;
				for (int j = 0; j < V; j++)
				{
					if (Result[i][j])
					{
						Result[i][j] = 0;
						Result[j][i] = 0;
					}
				}
				break;
			}
		}
	}
}

void swap(int *a, int *b)
{
	int c = *a;
	*a = *b;
	*b = c;
	return;
}

void sort(int i)
{
	for (int j = 1; j < 3; j++)
	{
		for (int k = j + 1; k <= 3; k++)
		{
			if (req[i][k] < req[i][j])
			{
				//swap(req[i][k],req[i][j]);
				int temp = req[i][k];
				req[i][k] = req[i][j];
				req[i][j] = temp;
			}
		}
	}
}

int checkGroom(int id, int from)
{
	for (int i = from; i < id; i++)
	{
		if (req[i][4] == 1)
		{
			if (req[i][0] == req[id][0] && req[i][1] == req[id][1] && req[i][2] == req[id][2] && req[i][3] == req[id][3])
				return i;
		}
	}
	return -1;
}

int main()
{
	int x = pow(2, (noOfRequests + 1));
	x = x - 2;
	//initialize network graph
  ifstream input("USBACKBONE.txt", ios::in);
	for (int i = 0; i < E; i++)
	{
		int j, k, l;
		input >> j >> k >> l;
		graph[j][k] = l;
		graph[k][j] = l;
	}
	shmid = shmget(IPC_PRIVATE, sizeof(f[V][V]), 0777 | IPC_CREAT);
	c = (f(*)[V])shmat(shmid, 0, 0);
	shmid1 = shmget(IPC_PRIVATE, 6 * sizeof(int), 0777 | IPC_CREAT);
	r = (int *)shmat(shmid1, 0, 0); //0-no of requests comp,1-no of req blocked,2-no of slots comp,3-no of slots block
	shmid2 = shmget(IPC_PRIVATE, 2 * sizeof(int), 0777 | IPC_CREAT);
	nor = (int *)shmat(shmid2, 0, 0); //no of requests
	shmid3 = shmget(IPC_PRIVATE, V * sizeof(int), 0777 | IPC_CREAT);
	source = (int *)shmat(shmid3, 0, 0);
	shmid4 = shmget(IPC_PRIVATE, sizeof(int[2048][15]), 0777 | IPC_CREAT);
	req = (int(*)[15])shmat(shmid4, 0, 0);
	shmid5 = shmget(IPC_PRIVATE, 2 * sizeof(int), 0777 | IPC_CREAT);
	number = (int *)shmat(shmid5, 0, 0);
	for (int i = 0; i < V; i++)
	{
		for (int j = 0; j < V; j++)
		{
			for (int k = 0; k < slots; k++)
				c[i][j].b[k] = 0;
		}
	}
	int noOfBlockedReq = 0, noOfFinishedReq = 0;
	for (int i = 0; i < noOfRequests; i++)
	{
		fork();
		int id = number[0];
		number[0]++;
		if (id == totalRequests)
		{
			sleep(15);
			/*for(int i=0;i<=id;i++)
			{
				for(int j=0;j<11;j++)
				{
					cout<<req[i][j]<<" "; /* 0-source
								 1,2,3 - destinations
								 4 - status
								 5 - beginning slot
								 6 - number of slots
								 7 - request#
								 8 - groomed or not ( 0 - not groomed,1 - groomed)
								 9 - request# with which it is groomed
								 10 - whether groomed at the beginning or at the end ( 1 - beginning, 2 - end)

						      
				}
				cout<<endl;
			}*/
			int noOfBlocked = 0;
			for (int i = 0; i <= id; i++)
			{
				if (req[i][4] == 2)
					noOfBlocked++;
			}
			float BP = (float)noOfBlocked / totalRequests;
			cout << id << " Blocked req = " << noOfBlocked << ", BP = " << BP << endl;
			cout << id << "  no of requests groomed = " << r[4] << endl;
			float BBP = (float)r[3] / (r[2] + r[3]);
			cout << id << "   "
					 << "BBP = " << BBP << endl;
		}
		if (id >= totalRequests)
			exit(-1);

		time_t now = time(0);
		dt = ctime(&now);
		pid = getpid();
		unsigned tt = unsigned(pid) + unsigned(time(0));
		srand(tt);
		int burstTime = 0;
		int src = 0;
		//cout<<"number"<<number[0]<<endl;

		int noOfSlotsReq = generateRequest(&burstTime, &src, id);
		req[id][6] = noOfSlotsReq;
		req[id][7] = id;
		int a[T];
		int temp = 0;
		for (int i = 0; i < V; i++)
		{
			//cout<<activePoint[i]<<" ";
			if (activePoint[i] == 1)
			{
				a[temp] = i;
				//cout<<endl<<i<<endl;
				temp++;
			}
		}
		dijkstra(src);
		remove();
		/*cout<<pid<<" ";
		for(int i=0;i<V;i++)
		{	
	    	for(int j=i+1;j<V;j++)
		    {
		        if(Result[i][j])
		            cout<<i<<"->"<<j<<",";
		    }
		}
		cout<<endl;*/
		sort(id);
		int from = 0;
		int temp2 = 0;
		while (from < id && temp2 == 0)
		{
			int groom = checkGroom(id, from);
			from = groom + 1;
			if (groom < 0)
				break;
			if (req[groom][10] == 1)
				continue;
			if (groom >= 0)
			{

				//cout<<"Request "<<id<<" can be groomed with request "<<groom<<endl;
				int bg = req[groom][5]; //beginning slot of the request with id "groom"
				int rs = req[id][6];		//slots required for the present request
				int flag1 = 0;
				if (req[id][6] <= bg)
				{
					//cout<<"begin"<<endl;
					for (int i = 0; i < V && flag1 == 0; i++)
					{
						for (int j = 0; j < V && flag1 == 0; j++)
						{
							if (Result[i][j] > 0)
							{
								for (int k = bg - 1; k >= bg - rs; k--)
								{
									if (c[i][j].b[k] != 0)
										flag1 = 1;
								}
							}
						}
					}
					if (flag1 == 0)
					{
						r[4]++;
						r[4]++;
						req[id][4] = 1;
						req[id][8] = 1;
						req[groom][8] = 1;
						req[id][9] = groom;
						req[groom][9] = id;
						req[id][10] = 2;
						req[groom][10] = 1;
						r[2] = r[2] + noOfSlotsReq;
						r[0]++;
						//int bg=req[groom][5];      //beginning slot of the request with id "groom"
						//int rs=req[id][6];
						for (int i = 0; i < V; i++)
						{
							for (int j = 0; j < V; j++)
							{
								if (Result[i][j] > 0)
								{
									for (int k = bg - 1; k >= bg - rs; k--)
									{
										c[i][j].b[k] = 1;
									}
								}
							}
						}
						req[id][5] = bg - rs;
						temp2 = 1;
						sleep(burstTime);
						if (req[groom][4] == 0)
						{
							for (int i = 0; i < V; i++)
							{
								for (int j = 0; j < V; j++)
								{
									if (Result[i][j])
									{
										for (int k = req[id][5]; k < (req[id][5] + req[id][6] + 2); k++)
											c[i][j].b[k] = 0;
									}
								}
							}
							req[id][4] = 0;
						}

						else
						{
							for (int i = 0; i < V; i++)
							{
								for (int j = 0; j < V; j++)
								{
									if (Result[i][j])
									{
										for (int k = req[id][5]; k < (req[id][5] + req[id][6]); k++)
											c[i][j].b[k] = 0;
									}
								}
							}
							req[id][4] = 0;
						}
					}
				}
				if (temp2 == 0)
				{
					//cout<<"end"<<endl;
					int es = bg + req[groom][6] + 1;
					int flag2 = 0;
					if ((req[id][6] + es) <= 319)
					{
						for (int i = 0; i < V && flag2 == 0; i++)
						{
							for (int j = 0; j < V && flag2 == 0; j++)
							{
								if (Result[i][j] > 0)
								{
									for (int k = es + 1; k <= es + rs; k++)
									{
										if (c[i][j].b[k] != 0)
											flag2 = 1;
									}
								}
							}
						}
						if (flag2 == 0)
						{
							r[4]++;
							r[4]++;
							req[id][4] = 1;
							req[id][8] = 1;
							req[groom][8] = 1;
							req[id][9] = groom;
							req[groom][9] = id;
							req[id][10] = 1;
							req[groom][10] = 2;
							r[2] = r[2] + noOfSlotsReq;
							r[0]++;
							//int bg=req[groom][5];      //beginning slot of the request with id "groom"
							//int rs=req[id][6];
							for (int i = 0; i < V; i++)
							{
								for (int j = 0; j < V; j++)
								{
									if (Result[i][j] > 0)
									{
										for (int k = es + 1; k <= es + rs; k++)
										{
											c[i][j].b[k] = 1;
										}
									}
								}
							}
							req[id][5] = es;
							temp2 = 1;
							sleep(burstTime);
							if (req[groom][4] == 0)
							{
								for (int i = 0; i < V; i++)
								{
									for (int j = 0; j < V; j++)
									{
										if (Result[i][j])
										{
											for (int k = req[id][5] - 1; k < (req[id][5] + req[id][6] + 2); k++)
												c[i][j].b[k] = 0;
										}
									}
								}
								req[id][4] = 0;
							}

							else
							{
								for (int i = 0; i < V; i++)
								{
									for (int j = 0; j < V; j++)
									{
										if (Result[i][j])
										{
											for (int k = req[id][5] + 1; k < (req[id][5] + req[id][6]); k++)
												c[i][j].b[k] = 0;
										}
									}
								}
								req[id][4] = 0;
							}
						}
					}
				}
			}
		}
		if (req[id][10] == 0)
		{

			int begin = 0;
			int ans = check(noOfSlotsReq + 2, &begin);
			if (ans == 1)
			{
				source[src]++;
				req[id][4] = 1;
				req[id][5] = begin;
				sleep(burstTime);

				r[2] = r[2] + noOfSlotsReq + 2;
				//cout<<pid<<"  "<<begin<<"   ";
				int temp3 = 0;
				if (req[id][8] == 1)
				{
					int groom1 = req[id][9];
					temp3 = req[groom1][4];
				}
				if (req[id][8] == 0 || temp3 == 0)
				{
					int o = begin;
					int v = 0, x = 0;
					for (int i = 0; i < V; i++)
					{
						begin = o;
						for (int j = 0; j < V; j++)
						{
							begin = o;
							if (Result[i][j] > 0)
							{
								c[i][j].status--;
								while (begin < noOfSlotsReq + o + 2)
								{
									c[i][j].b[begin] = 0;
									//cout<<c[i][j].b[begin];
									begin++;
								}
							}
						}
					}
				}
				else
				{
					int groom = req[id][9];
					if (req[id][10] == 1)
					{
						for (int i = 0; i < V; i++)
						{
							for (int j = 0; j < V; j++)
							{
								if (Result[i][j])
								{
									for (int k = req[id][5] + 2; k < (req[id][5] + req[id][6] + 2); k++)
										c[i][j].b[k] = 0;
								}
							}
						}
					}
					if (req[id][10] == 2)
					{
						for (int i = 0; i < V; i++)
						{
							for (int j = 0; j < V; j++)
							{
								if (Result[i][j])
								{
									for (int k = req[id][5]; k < (req[id][5] + req[id][6]); k++)
										c[i][j].b[k] = 0;
								}
							}
						}
					}
				}
				req[id][4] = 0;
				source[src]--;
				r[0]++;
				//cout<<"Request Completed "<<r[0]<<" "<<r[1]<<"   "<<r[2]<<" "<<r[3]<<endl;
				//cout<<src<<"  "<<source[src]<<endl;
				int k = rand() % 5;
				sleep(k);
				//cout<<endl;
				/*for(int i=0;i<slots;i++)
					cout<<c[0][1].b[i];
				cout<<endl;*/
			}
			else
			{
				//for(int i=0;i<slots;i++)
				//cout<<c[0][1].b[i];
				req[id][4] = 2;
				r[3] += noOfSlotsReq;
				r[1]++;
				req[id][7] = id;
				//cout<<pid<<"Request Blocked "<<r[1]<<"   "<<r[3]<<endl;
				int k = rand() % 5;
				//cout<<id<<" "<<k<<endl;
				sleep(k);
				//cout<<endl;
			}
		}
		//cout<<id<<endl;
		//cout<<"id"<<id<<endl;
		cout << id << " ";
		for (int j = 0; j < 11; j++)
			cout << req[id][j] << " ";
		cout << endl;

		//float BBP=(float)r[3]/(r[2]+r[3]);
		//cout<<pid<<"   "<<"BBP = "<<BBP<<endl;*/
	}
	return 0;
}
