/*
1. What is burst time per request?
2. 
*/

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
#include <signal.h>
#include <stdio.h> /* printf, scanf, puts, NULL */
#include <time.h>
using namespace std;

#define T 4 //not used
#define noOfRequests 9
#define totalRequests 477
#define maxSlotsPerReq 4
#define maxBtPerReq 5
#define NO_OF_DEST 3
#define slots 292
#define V 28
#define E 44

int pid;
char *dt;
int *nor;
int *r;
int *number;
pid_t getpid(void);
int dist[V];
int Tgraph[T][T];
int TMst[T][T];
int Result[V][V];   // ???????????????????
int activePoint[V]; // keeps track of active vertices in the network for the request(source and destinations),
                    // helps in randomly assigning destinations
int graph[V][V];
int *source;
int (*req)[15]; //stores information about a request

struct f // slot matrix
{
  bool b[slots];
  int status;
};
f (*c)[V]; //c is a pointer to an array of V=28 structures

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
  /* */
  for (int i = 0; i < V; i++)
    activePoint[i] = 0;    //
  int source = rand() % V; // randomly select source
  req[id][0] = source;
  *src = source;
  activePoint[source] = 1;

  for (int i = 0; i < NO_OF_DEST; i++) //randomly assigning destinations here
  {
    int dest = rand() % V;
    while (activePoint[dest] == 1)
      dest = rand() % V;
    req[id][i + 1] = dest;
    activePoint[dest] = 1;
  }

  int k = (2 * (V - 1) - 1) - (2 * NO_OF_DEST - 1); // this is not even being used
  int noOfSlotsReq = (rand() % maxSlotsPerReq) + 1; // randomly allot slots between 1 and 4
  int burstTime = (rand() % maxBtPerReq) + 1;       // randomly assign burst time between 1 and 5
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
      if (!sptSet[v] && graph[u][v] && dist[u] + graph[u][v] < dist[v])
      {
        parent[v] = u;
        dist[v] = dist[u] + graph[u][v];
      }
  }
  printSolution(parent); //adds to result
}

int check(int r, int *begin) // checks if slots are available in the slot matrix for allocation
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
  //cout<<"m = "<<m<<"  n = "<<n<<" "<<endl;
  int f = 0;
  flag = 0;
  /*for(int i=0;i<slots;i++)
				cout<<c[0][7].b[i];
			cout<<endl;*/
  while (f < slots && k < r)
  {
    //cout<<"f = "<<f<<" k = "<<k<<endl;
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
              //cout<<"first Else "<<"i="<<i<<" j="<<j<<" f="<<f<<endl;
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
      //cout<<"Last Else"<<endl;
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
        //cout<<"f = "<<f<<endl;
        if (Result[i][j])
        {

          //cout<<"f = "<<f<<"   "<<Result[i][j]<<endl;
          c[i][j].status++;
          for (g = f - r; g < f; g++)
          {
            // cout<<"i = "<<i<<" j = "<<j<<" g = "<<g<<endl;
            c[i][j].b[g] = 1;
          }
        }
      }
    }
    return 1; //slots are available and has been allocated
  }
  else
    return 0; //no slots are available, request will be blocked
}

void path()
{
  for (int i = 0; i < V; i++)
    for (int j = i + 1; j < V; j++)
      if (Result[i][j])
        cout << i << "-->" << j << ", ";
  cout << endl;
}

void remove()
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

void sort(int i) // what does this sort?
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

//-----------------driver function--------------------------------------------//
int main()
{

  //-----READS graph from text file-------------------------
  ifstream input("USBACKBONE.txt", ios::in);
  for (int i = 0; i < E; i++)
  {
    int j, k, l;
    input >> j >> k >> l;
    graph[j][k] = l;
    graph[k][j] = l;
  }
  //----- initializes shared memory----------------------

  //attaches slot matrix to shared memory, which is of size [28][28] where 28 is the no of vertices
  int shmid = shmget(IPC_PRIVATE, sizeof(f[V][V]), 0777 | IPC_CREAT); //size of f[V][V]
  c = (f(*)[V])shmat(shmid, 0, 0);                                    //attaches structure f as a shared memory

  //attaches r to shared memory, r stores information about the requests for the whole process
  int shmid1 = shmget(IPC_PRIVATE, 4 * sizeof(int), 0777 | IPC_CREAT);
  r = (int *)shmat(shmid1, 0, 0); //0-no of requests comp,1-no of req blocked,2-no of slots comp,3-no of slots block
  // r is used at the end when all requests are run to calculate the BP, BBP

  // what is the use of source variable ?
  int shmid3 = shmget(IPC_PRIVATE, V * sizeof(int), 0777 | IPC_CREAT);
  source = (int *)shmat(shmid3, 0, 0);

  //req
  int shmid4 = shmget(IPC_PRIVATE, sizeof(int[10000][15]), 0777 | IPC_CREAT);
  req = (int(*)[15])shmat(shmid4, 0, 0);
  // 0-source, 1,2,3 - destinations, 4 - status, 5 - beginning slot, 6 - number of slots 7 - request#

  // stores the request count
  int shmid5 = shmget(IPC_PRIVATE, 2 * sizeof(int), 0777 | IPC_CREAT);
  number = (int *)shmat(shmid5, 0, 0);

  // for each element of matrix c[28][28], changing its spectrum allocation 320 slots to intial
  // value 0.
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
    //cout << "NOR: " << i << endl;
    fork();
    // cout << "------------------------" << endl;
    int id = number[0];
    number[0]++;
    if (id == totalRequests) //on the last request, display BP and BBP
    {
      sleep(15);
      /*
        0-source, 
        1,2,3 - destinations, 
        4 - status(1-ok, 2 - blocked), 
        5 - beginning slot,  
        6 - number of slots
        7 - no of requests 
      */
      int noOfBlocked = 0;
      for (int i = 0; i <= id; i++)
      {
        if (req[i][4] == 2)
          noOfBlocked++;
      }
      float BP = (float)noOfBlocked / totalRequests;
      cout << id << " Blocked req = " << noOfBlocked << ", BP = " << BP << endl;
      float BBP = (float)r[3] / (r[2] + r[3]);
      cout << id << "   "
           << "BBP = " << BBP << endl;
      ofstream myfile;
      myfile.open("dataset.txt", ios_base::app);
      myfile << V << "," << E << "," << totalRequests << "," << NO_OF_DEST << "," << slots << "," << BP << "," << BBP << endl;
      myfile.close();
    }

    if (id >= totalRequests) //when we have generated 600 requests, terminate calling process
    {
      kill(getpid(), SIGKILL);
      //exit(0);
    }

    // -----when id < totalRequests, Common to every request-----------------------
    time_t now = time(0);
    dt = ctime(&now);
    pid = getpid();
    unsigned tt = unsigned(pid) + unsigned(time(0));
    srand(tt);
    int burstTime = 0;
    int src = 0;

    int noOfSlotsReq = generateRequest(&burstTime, &src, id);
    int a[T];
    int temp = 0;
    for (int i = 0; i < V; i++)
    {
      if (activePoint[i] == 1)
      {
        a[temp] = i;
        temp++;
      }
    }
    dijkstra(src);
    remove();
    sort(id);
    int begin = 0;
    int requestStatus = check(noOfSlotsReq + 2, &begin);
    if (requestStatus == 1) // slots successfully allocated for the request
    {
      source[src]++;
      req[id][4] = 1; // 1 means request is active
      req[id][5] = begin;
      req[id][6] = noOfSlotsReq;
      req[id][7] = id;
      sleep(burstTime);
      req[id][4] = 0;           // 0 means completed
      r[2] += noOfSlotsReq + 2; // r[2] : no of slots allocated
      int o = begin;
      int v = 0, x = 0;
      for (int i = 0; i < V; i++) // I GUESS slots deallocation takes place from the slot matrix
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
              begin++;
            }
          }
        }
      }
      source[src]--;
      r[0]++; // r[0] : no of requests completed
      int k = rand() % 3;
      sleep(k); // after this request, process sleeps for a random amount of time before spawning another process/request
    }
    else //no slots available, request has been blocked
    {
      req[id][4] = 2;       //2 means blocked
      r[3] += noOfSlotsReq; // r[3] : no of slots blocked
      r[1]++;               // r[1] : no of requests blocked
      req[id][7] = id;
      cout << id << "  "
           << "Request Blocked" << endl;
      sleep(3); // WHY are we sleeping here after request is getting blocked???
    }
    /*
      0-source, 
      1,2,3 - destinations, 
      4 - status(0 - completed, 1 - active, 2 - blocked), 
      5 - beginning slot,  
      6 - number of slots
      7 - no of requests 
    */
    /*     cout << id << " source: " << req[id][0] << " dest: (" << req[id][1] << "," << req[id][2] << "," << req[id][3] << ") status: "
         << req[id][4] << " beg slot: " << req[id][5] << " no of slots: " << req[id][6] << " no of requets: " << req[id][7] << endl;
    */
    /*for (int j = 0; j < 8; j++)
      cout << req[id][j] << " ";
    cout << endl;
    */
  }
  return 0;
}
