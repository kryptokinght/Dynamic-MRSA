#include <bits/stdc++.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
using namespace std;

//VARIABLES
#define T 4 //not used
#define noOfRequests 10
#define totalRequests 5
#define maxSlotsPerReq 4
#define maxBtPerReq 5
#define NO_OF_DEST 3
#define SLOTS 10
#define VERTICES 6
#define EDGES 8
#define filename "stand6.txt"
#define output_file 5
//VARIABLES

int pid;
int *simulation_info;
int *number;
int Result[VERTICES][VERTICES];     // Stores the edges of the light tree
int secondPath[VERTICES][VERTICES]; // Stores the edges of the backup disjoint path
int activePoint[VERTICES];          // keeps track of active vertices in the network for the request(source and destinations),
                                    // helps in randomly assigning destinations
int graph[VERTICES][VERTICES];
int (*requestsInfo)[15]; /* stores information about a request
                          0-source, 
                          1,2,3 - destinations, 
                          4 - status(1-ok, 2 - blocked), 
                          5 - beginning slot,  
                          6 - number of SLOTS
                          7 - no of requests 
                        */

struct slotType // slot matrix
{
  int slots[SLOTS];
  //bool slots[SLOTS];
  int status;
};
slotType (*slotMatrix)[VERTICES]; //slotMatrix is a pointer to an array of VERTICES=28 structures

int generateRequest(int *bt, int *src, int requestId)
{
  /* Generates a request and returns the number of SLOTS for the request */
  for (int i = 0; i < VERTICES; i++)
    activePoint[i] = 0;           //
  int source = rand() % VERTICES; // randomly select source
  requestsInfo[requestId][0] = source;
  *src = source;
  activePoint[source] = 1;

  for (int i = 0; i < NO_OF_DEST; i++) //randomly assigning destinations here
  {
    int dest = rand() % VERTICES;
    while (activePoint[dest] == 1)
      dest = rand() % VERTICES;
    requestsInfo[requestId][i + 1] = dest;
    activePoint[dest] = 1;
  }
  int noOfSlotsReq = (rand() % maxSlotsPerReq) + 1; // randomly allot SLOTS between 1 and 4
  int burstTime = (rand() % maxBtPerReq) + 1;       // randomly assign burst time between 1 and 5
  *bt = burstTime;
  return noOfSlotsReq;
}

int addToResult(int parent[], int Result[VERTICES][VERTICES]) // what is parent?
{
  for (int i = 0; i < VERTICES; i++)
  {
    if (parent[i] != -1)
    {
      Result[i][parent[i]] = graph[i][parent[i]];
      Result[parent[i]][i] = graph[parent[i]][i];
    }
  }
}

int minDistance(int dist[], bool sptSet[])
{
  int min = INT_MAX, min_index;
  for (int v = 0; v < VERTICES; v++)
    if (sptSet[v] == false && dist[v] <= min)
      min = dist[v], min_index = v;
  return min_index;
}

void dijkstra(int src, int Result[VERTICES][VERTICES])
{
  int parent[VERTICES];
  bool sptSet[VERTICES];
  int dist[VERTICES];
  for (int i = 0; i < VERTICES; i++) //intial values
  {
    parent[i] = -1;
    dist[i] = INT_MAX;
    sptSet[i] = false;
  }
  dist[src] = 0;
  for (int count = 0; count < VERTICES - 1; count++)
  {
    int u = minDistance(dist, sptSet);
    sptSet[u] = true;
    for (int v = 0; v < VERTICES; v++)
      if (!sptSet[v] && graph[u][v] && dist[u] + graph[u][v] < dist[v])
      {
        parent[v] = u;
        dist[v] = dist[u] + graph[u][v];
      }
  }
  for (int i = 0; i < VERTICES; i++)
  {
    cout << i << " :" << parent[i] << ", ";
  }
  cout << endl;
  addToResult(parent, Result); //adds to result
}

void secondary_Dijkstra(int src, int Result[VERTICES][VERTICES], int secondPath[VERTICES][VERTICES])
{
  int parent[VERTICES];
  bool sptSet[VERTICES];
  int dist[VERTICES];
  for (int i = 0; i < VERTICES; i++) //intial values
  {
    parent[i] = -1;
    dist[i] = INT_MAX;
    sptSet[i] = false;
  }
  dist[src] = 0;
  for (int count = 0; count < VERTICES - 1; count++)
  {
    int u = minDistance(dist, sptSet);
    sptSet[u] = true;
    for (int v = 0; v < VERTICES; v++)
      if (!sptSet[v] && graph[u][v] && dist[u] + graph[u][v] < dist[v])
      {
        parent[v] = u;
        dist[v] = dist[u] + graph[u][v];
      }
  }
  for (int i = 0; i < VERTICES; i++)
  {
    cout << i << " :" << parent[i] << ", ";
  }
  cout << endl;
  addToResult(parent, Result); //adds to result
}

int allocateSlots(int r, int *begin, int slot_val)
{
  /*
  Checks if SLOTS are avaialable in the slot matrix for allocation, if yes then allocates the slots in the matrix
  Returns 1 if slots allocated successfully
  Returns 0 if slots not allocated successfully
  */
  int k = 0;
  int m = 0, n = 0, flag = 0;

  for (m = 0; m < VERTICES; m++)
  {
    for (n = 0; n < VERTICES; n++)
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

  while (f < SLOTS && k < r)
  {
    if (slotMatrix[m][n].slots[f] == 0)
    {

      for (int i = m; i < VERTICES; i++)
      {
        for (int j = n + 1; j < VERTICES; j++)
        {
          if (Result[i][j])
          {
            if (slotMatrix[i][j].slots[f] == 0)
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
      //cout<<"Last Else"<<endl;
    }
  }
  cout << "Slots reqd: " << r << endl;
  if (k >= r)
  {
    int g = f - r;
    *begin = g;
    for (int i = 0; i < VERTICES; i++)
    {
      for (int j = 0; j < VERTICES; j++)
      {
        //cout<<"f = "<<f<<endl;
        if (Result[i][j])
        {

          //cout<<"f = "<<f<<"   "<<Result[i][j]<<endl;
          slotMatrix[i][j].status++;
          for (g = f - r; g < f; g++)
          {
            //cout << pid << " i = " << i << " j = " << j << " g = " << g << endl;
            slotMatrix[i][j].slots[g] = slot_val + 1;
          }
        }
      }
    }
    return 1; //SLOTS are available and has been allocated
  }
  else
    return 0; //no SLOTS are available, request will be blocked
}

void path()
{
  for (int i = 0; i < VERTICES; i++)
    for (int j = i + 1; j < VERTICES; j++)
      if (Result[i][j])
        cout << i << "-->" << j << ", ";
  cout << endl;
}

void remove() //I don't know what it is removing, changing some values of Result back to 0
{
  int flag = 0;
  while (flag == 0)
  {
    flag = 1;
    for (int i = 0; i < VERTICES; i++)
    {
      if (activePoint[i] == 1)
        continue;
      int temp = 0;
      for (int j = 0; j < VERTICES; j++)
      {
        if (Result[i][j])
          temp++;
      }
      if (temp == 1)
      {
        flag = 0;
        for (int j = 0; j < VERTICES; j++)
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

void sort(int id) // what does this sort?
{
  /* */
  for (int j = 1; j < NO_OF_DEST; j++)
  {
    for (int k = j + 1; k <= NO_OF_DEST; k++)
    {
      if (requestsInfo[id][k] < requestsInfo[id][j])
      {
        int temp = requestsInfo[id][k];
        requestsInfo[id][k] = requestsInfo[id][j];
        requestsInfo[id][j] = temp;
      }
    }
  }
}

//-----------------driver function--------------------------------------------//
int main()
{
  //-----READS graph from text file-------------------------
  ifstream input(filename, ios::in);
  for (int i = 0; i < EDGES; i++)
  {
    int j, k, l;
    input >> j >> k >> l;
    graph[j][k] = l;
    graph[k][j] = l;
  }
  //----- initializes shared memory----------------------
  //attaches slot matrix to shared memory, which is of size [28][28] where 28 is the no of vertices
  int shmid = shmget(IPC_PRIVATE, sizeof(slotType[VERTICES][VERTICES]), 0777 | IPC_CREAT); //size of f[VERTICES][VERTICES]
  slotMatrix = (slotType(*)[VERTICES])shmat(shmid, 0, 0);                                  //attaches structure f as a shared memory
  //slotMatrix[0][0].status = 1;
  //cout << sizeof(slotMatrix[0][0].status) << endl;

  //attaches simulation_info to shared memory, simulation_info stores information about the requests for the whole process
  int shmid1 = shmget(IPC_PRIVATE, 4 * sizeof(int), 0777 | IPC_CREAT);
  simulation_info = (int *)shmat(shmid1, 0, 0); //0-no of requests comp,1-no of requestsInfo blocked,2-no of SLOTS comp,3-no of SLOTS block
  // simulation_info is used at the end when all requests are run to calculate the BP, BBP

  //requestsInfo
  int shmid4 = shmget(IPC_PRIVATE, sizeof(int[10000][15]), 0777 | IPC_CREAT);
  requestsInfo = (int(*)[15])shmat(shmid4, 0, 0);
  // 0-source, 1,2,3 - destinations, 4 - status, 5 - beginning slot, 6 - number of SLOTS 7 - request#

  // stores the request count
  int shmid5 = shmget(IPC_PRIVATE, 2 * sizeof(int), 0777 | IPC_CREAT);
  number = (int *)shmat(shmid5, 0, 0);

  // for each element of matrix slotMatrix[28][28], changing its spectrum allocation 320 SLOTS to intial
  // value 0.
  for (int i = 0; i < VERTICES; i++)
  {
    for (int j = 0; j < VERTICES; j++)
    {
      for (int k = 0; k < SLOTS; k++)
        slotMatrix[i][j].slots[k] = 0;
    }
  }
  cout << "Shared memory allocated successfully!" << endl;
  int noOfBlockedReq = 0, noOfFinishedReq = 0;
  for (int i = 0; i < noOfRequests; i++)
  {
    fork();
    int id = number[0];
    number[0]++;
    if (id == totalRequests) //on the last request, display BP and BBP
    {
      sleep(15);
      int noOfBlocked = 0;
      for (int i = 0; i <= id; i++)
      {
        if (requestsInfo[i][NO_OF_DEST + 1] == 2)
          noOfBlocked++;
      }
      float BP = (float)noOfBlocked / totalRequests;
      cout << id << " Blocked requests = " << noOfBlocked << ", BP = " << BP << endl;
      float BBP = (float)simulation_info[3] / (simulation_info[2] + simulation_info[3]);
      cout << id << "   "
           << "BBP = " << BBP << endl;
      /*
      ofstream myfile;
      myfile.open("dataset1.txt", ios_base::app);
      myfile << VERTICES << "," << EDGES << "," << totalRequests << "," << NO_OF_DEST << "," << SLOTS << "," << BP << "," << BBP << endl;
      myfile.close();
      */
    }

    if (id >= totalRequests) //when we have generated 600 requests, terminate calling process
    {
      kill(getpid(), SIGKILL);
      //exit(-1);
    }

    // -----when id < totalRequests, Common to every request-----------------------
    pid = getpid();
    unsigned tt = unsigned(pid) + unsigned(time(0));
    srand(tt); //seeds random
    int burstTime = 0;
    int src = 0;

    int noOfSlotsReq = generateRequest(&burstTime, &src, id); //burstTime and src is passed by ref, gets updated and send
    cout << "Reqq id: " << id << " Src: " << src << " Dst: {" << requestsInfo[id][1] << "," << requestsInfo[id][2] << "," << requestsInfo[id][3] << "} slots: " << noOfSlotsReq << endl;
    dijkstra(src, Result);
    remove();
    sort(id); // sort the destinations in requests according to their value
    int begin = 0;
    int isSlotsAllocated = allocateSlots(noOfSlotsReq + 2, &begin, id);

    if (isSlotsAllocated == 1) // SLOTS successfully allocated for the request
    {
      path(); //print the path
      ofstream myfile;
      string filenam = "samples/slot_" + to_string(id) + ".txt";
      myfile.open(filenam, ios_base::app);
      //myfile << VERTICES << "," << EDGES << "," << totalRequests << "," << NO_OF_DEST << "," << SLOTS << "," << BP << "," << BBP << endl;
      myfile << "Reqq id: " << id << " Src: " << src << " Dst: {" << requestsInfo[id][1] << "," << requestsInfo[id][2] << "," << requestsInfo[id][3] << "} slots: " << noOfSlotsReq << endl;
      //printing the slot

      myfile << "    ";
      for (int i = 1; i <= SLOTS; i++)
        myfile << i << " ";
      myfile << endl;
      for (int i = 0; i < VERTICES; i++)
      {
        for (int j = 0; j < VERTICES; j++)
        {
          if (j > i && graph[i][j] != 0)
          {
            myfile << i << "-" << j << ": ";
            for (int k = 0; k < SLOTS; k++)
            {
              myfile << slotMatrix[i][j].slots[k] << " ";
            }
            myfile << endl;
          }
        }
      }
      myfile.close();
      //end of slot print

      requestsInfo[id][NO_OF_DEST + 1] = 1; // 1 means request is active
      requestsInfo[id][NO_OF_DEST + 2] = begin;
      requestsInfo[id][NO_OF_DEST + 3] = noOfSlotsReq;
      requestsInfo[id][NO_OF_DEST + 4] = id;
      sleep(burstTime);
      requestsInfo[id][NO_OF_DEST + 1] = 0;   // 0 means completed
      simulation_info[2] += noOfSlotsReq + 2; // simulation_info[2] : no of SLOTS allocated
      int o = begin;
      int v = 0, x = 0;
      for (int i = 0; i < VERTICES; i++) // SLOTS deallocation takes place from the slot matrix
      {
        //begin = o;
        for (int j = 0; j < VERTICES; j++)
        {
          begin = o;
          if (Result[i][j] > 0)
          {
            slotMatrix[i][j].status--;
            while (begin < noOfSlotsReq + o + 2)
            {
              slotMatrix[i][j].slots[begin] = 0;
              begin++;
            }
          }
        }
      }
      simulation_info[0]++; // simulation_info[0] : no of requests completed
      int k = rand() % 3;
      sleep(k); // after this request, process sleeps for a random amount of time before spawning another process/request
    }
    else //no SLOTS available, request has been blocked
    {
      //requestsInfo[id][4] = 2;            //2 means blocked
      requestsInfo[id][NO_OF_DEST + 1] = 2;
      simulation_info[3] += noOfSlotsReq; // simulation_info[3] : no of SLOTS blocked
      simulation_info[1]++;               // simulation_info[1] : no of requests blocked
      //requestsInfo[id][7] = id;
      requestsInfo[id][NO_OF_DEST + 4] = id;
      cout << id << "  "
           << "Request Blocked" << endl;
      sleep(3); // WHY are we sleeping here after request is getting blocked???
    }
    /*     cout << id << " source: " << requestsInfo[id][0] << " dest: (" << requestsInfo[id][1] << "," << requestsInfo[id][2] << "," << requestsInfo[id][3] << ") status: "
         << requestsInfo[id][4] << " beg slot: " << requestsInfo[id][5] << " no of SLOTS: " << requestsInfo[id][6] << " no of requets: " << requestsInfo[id][7] << endl;
    */
    /*for (int j = 0; j < 8; j++)
      cout << requestsInfo[id][j] << " ";
    cout << endl;
    */
  }
  return 0;
}
