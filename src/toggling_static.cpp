#include <bits/stdc++.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/wait.h>

using namespace std;

//VARIABLES
#define T 4 //not used
#define noOfRequests 10
#define totalRequests 200
#define maxSlotsPerReq 4
#define maxBtPerReq 20
#define NO_OF_DEST 3
#define SLOTS 1000
#define VERTICES 14
#define EDGES 22
#define filename "./NSFNET.txt"
#define output_file 5
#define randomFactor 50//VARIABLES

int pid;
int *simulation_info;
int *number;
float *FI_array;

// int Result[VERTICES][VERTICES];     // Stores the edges of the light tree
int secondPath[VERTICES][VERTICES]; // Stores the edges of the backup disjoint path
int activePoint[VERTICES];          // keeps track of active vertices in the network for the request(source and destinations),
                                    // helps in randomly assigning destinations
int graph[VERTICES][VERTICES];
int (*requestsInfo)[15]; /* stores information about a request
                          0-source, 
                          1,2,3 - destinations, 
                          NO OF DEST +1 - status(1-active, 0-completed, 2 - blocked), 
                          NO OF DEST +2 - beginning slot original,  
                          NO OF DEST +3 - begininning slot bakcup
                          NO OF DEST +4 - required slots
                          NO OF DEST +5 - burst time 
                          NO OF DEST +6 - req_id
                          NO OF DEST +7 - reason of blocking
                          NO OF DEST +8 - hop count primary
                          NO OF DEST +9 - hop count backup
                        */
enum blocking
{
    lightTreeNotFound,
    UnableToAllocateSlots
};

int id_shm;
sem_t *id_semaphore;

int mat_shm;
sem_t *matrix_semaphore;

struct slotType // slot matrix
{
    int slots[SLOTS];
    //bool slots[SLOTS];
    int status;
};


slotType (*slotMatrix)[VERTICES]; //slotMatrix is a pointer to an array of VERTICES

slotType slotMatrixStates[totalRequests][VERTICES][VERTICES];
int reqSatisfied[totalRequests];

int generateRequest(int &bt, int &src, int requestId)
{
    /* Generates a request and returns the number of SLOTS for the request */
    for (int i = 0; i < VERTICES; i++)
        activePoint[i] = 0;         //
    int source = rand() % VERTICES; // randomly select source
    requestsInfo[requestId][0] = source;
    src = source;
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
    bt = burstTime;
    requestsInfo[requestId][NO_OF_DEST + 5]=burstTime;
    return noOfSlotsReq;
}
void printGraph(int graph[VERTICES][VERTICES]){
    for(int i=0;i<VERTICES;i++){
        for(int j=0;j<VERTICES;j++){
            cout<<graph[i][j]<<" ";
        }
        cout<<endl;
    }
}

set<pair<int, int>> getLightTree(int parent[], int src, int destinations[])
{

    set<pair<int, int>> s;
    // cout << endl;
    for (int i = 0; i < NO_OF_DEST; i++)
    {
        int dest = destinations[i];
        int par = parent[dest];
        if (par == -1){
            s.clear();
            return s;
        }
        s.insert({par, dest});
        // cout << "(" << par << "-" << dest << ")"
        //      << " ";

        while (par != src)
        {
            dest = par;
            par = parent[dest];
            if (par == -1){
                s.clear();
                return s;
            }
            s.insert({par, dest});
            // cout << "(" << par << "-" << dest << ")"
            //      << " ";
        }
        // cout << endl;
    }
    return s;
}

int minDistance(int dist[], bool sptSet[])
{
    int min = INT_MAX, min_index;
    for (int v = 0; v < VERTICES; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;
    return min_index;
}

set<pair<int, int>> dijkstra(int src, int destinations[])
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
    // cout << "ORIGINAL PARENT" << endl;
    // for (int i = 0; i < VERTICES; i++)
    // {
    //     cout << i << " :" << parent[i] << ", ";
    // }
    // cout << endl;
    set<pair<int, int>> s = getLightTree(parent, src, destinations); //adds to result
        
    cout << "ORIGINAL LIGHT TREE" << endl;
    for (auto it : s)
    {
        int u = it.first;
        int v = it.second;
        cout<<"("<<u<<"-"<<v<<")"<<",";
    }
    cout << endl;

    return s;
}

set<pair<int, int>> secondary_dijkstra(int src, int destinations[], set<pair<int, int>> lightTree)
{
    int graph_clone[VERTICES][VERTICES];
    for (int i = 0; i < VERTICES; i++)
    {
        for (int j = 0; j < VERTICES; j++)
        {
            graph_clone[i][j] = graph[i][j];
        }
    }

    for (auto it : lightTree)
    {
        int first = it.first;
        int second = it.second;
        graph_clone[first][second] = 0;
        graph_clone[second][first] = 0;
    }

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
        {
            if (!sptSet[v] && graph_clone[u][v] && dist[u] + graph_clone[u][v] < dist[v])
            {
                parent[v] = u;
                dist[v] = dist[u] + graph_clone[u][v];
            }
        }
    }

    // cout << "BACKUP PARENT" << endl;
    // for (int i = 0; i < VERTICES; i++)
    // {
    //     cout << i << " :" << parent[i] << ", ";
    // }
    // cout << endl;
    set<pair<int, int>> s = getLightTree(parent, src, destinations); //adds to result
    
    cout << "BACKUP LIGHT TREE" << endl;
    for (auto it : s)
    {
        int u = it.first;
        int v = it.second;
        cout<< "("<<u<<"-"<<v<<")"<<",";
    }
    cout << endl;
    return s;
}


//=============================PRINT UTILITY FUNCTION FOR MAIN SLOT MATRIX===================================//

void printMainSlotMatrix()
{
    for (int i = 1; i <= SLOTS; i++)
        cout << i << "   ";
    cout << endl;
    for (int i = 0; i < VERTICES; i++)
    {
        for (int j = 0; j < VERTICES; j++)
        {

            cout << i << "-" << j << ":\t ";
            for (int k = 0; k < SLOTS; k++)
            {
                cout << slotMatrix[i][j].slots[k] << "   ";
            }
            cout << endl;
        }
    }
}

//=============================PRINT UTILITY FUNCTION FOR MAIN SLOT MATRIX===================================//

void printSlotMatrixState(int req_id){
    
    cout<<"Printing Slot Matrix State"<<endl;
    for (int i = 1; i <= SLOTS; i++)
        cout << i << "   ";
    cout<<endl;
    for (int i = 0; i < VERTICES; i++)
    {
        for (int j = 0; j < VERTICES; j++)
        {
            cout << i << "-" << j << ":\t ";
            for (int k = 0; k < SLOTS; k++)
            {
                cout<<slotMatrixStates[req_id][i][j].slots[k]<<"   ";
            }
            cout<<endl;
        }
    }
    cout<<endl;
}

void writeToFile()
{
    ofstream myfile;
    string filenam = "samples/slot_matrices_proposed.txt";
    myfile.open(filenam, ios_base::out);
    //myfile << VERTICES << "," << EDGES << "," << totalRequests << "," << NO_OF_DEST << "," << SLOTS << "," << BP << "," << BBP << endl;
    //printing the slot

    //   0-source,
    //   1,2,3 - destinations,
    //   NO OF DEST +1 - status(1-active, 3- groomed, 4-completed, 2 - blocked),
    //   NO OF DEST +2 - beginning slot original,
    //   NO OF DEST +3 - begininning slot bakcup
    //   NO OF DEST +4 - required slots
    //   NO OF DEST +5 - burst time
    //   NO OF DEST +6 - req_id
    //   NO OF DEST +7 - reason of blocking
    for (int req_id = 0; req_id < totalRequests; req_id++)
    {
        myfile << "Reqq_id: " << req_id << " Src: " << requestsInfo[req_id][0];
        myfile << " Dst: {";
        for (int i = 0; i < NO_OF_DEST; i++)
        {
            myfile << requestsInfo[req_id][i + 1] << ",";
        }

        myfile << " } Slots_Req: " << requestsInfo[req_id][NO_OF_DEST + 4] << " Burst_Time: " << requestsInfo[req_id][NO_OF_DEST + 5];
        myfile << " Status: " << requestsInfo[req_id][NO_OF_DEST + 1] << " beg_slot_orig: " << requestsInfo[req_id][NO_OF_DEST + 2] + 1 << " beg_slot_index_backup: " << requestsInfo[req_id][NO_OF_DEST + 3] + 1;
        if (requestsInfo[req_id][NO_OF_DEST + 7] == lightTreeNotFound)
            myfile << " Reason_of_Blocking: Light Tree not found " << endl;
        else if (requestsInfo[req_id][NO_OF_DEST + 7] == UnableToAllocateSlots)
            myfile << " Reason_of_Blocking: Unable to allocate " << endl;
        else
            myfile << endl;

        myfile << "SLOT MATRIX \n";
        myfile << "\t\t";
        for (int i = 1; i <= SLOTS; i++)
            myfile << i << "   ";

        myfile << endl;
        for (int i = 0; i < VERTICES; i++)
        {
            for (int j = 0; j < VERTICES; j++)
            {

                myfile << i << "-" << j << ":\t ";
                for (int k = 0; k < SLOTS; k++)
                {
                    myfile << slotMatrixStates[req_id][i][j].slots[k] << "\t";
                }
                myfile << endl;
            }
        }
    }
    myfile.close();
    //end of slot print
}


//=============================DEALLOCATION OF MAIN SLOT MATRIX =============================================//
void deallocationMainSlotMatrix(set<pair<int, int>> lightTree, int beginIndex, int requiredSlots){
    // deallocation
    for (auto it : lightTree)
    {
        int u = it.first;
        int v = it.second;
        int lastSlot = beginIndex + requiredSlots - 1;
        for (int i = beginIndex; i <= lastSlot; i++)
        {
            slotMatrix[u][v].slots[i] = 0;
        }
    }
}

//=============================FINDING FI======================================//
float findFIMainSlotMatrix(set<pair<int, int>> lightTree)
{
    //finding the FI
    float FI=0;
    for (auto it : lightTree)
    {
        int u = it.first;
        int v = it.second;
        int noOfEmptySlots = 0;
        int noOfEmptyContigousSlots = 0;
        //finding the slots
        for (int i = 0; i < SLOTS;)
        {
            int k = i;
            while (k < SLOTS && slotMatrix[u][v].slots[k] == 0)
            {
                k++;
            }
            if (k - i > 0)
            {
                noOfEmptySlots += k - i;
                noOfEmptyContigousSlots++;
                i = k + 1;
            }
            else
            {
                i = i + 1;
            }
        }

        float FI_edge = float(noOfEmptyContigousSlots) / float(noOfEmptySlots);
        FI = FI + FI_edge;
    }

    return FI;
}
//=============================FINDING FI======================================//



//returns a pair of (slotFound and beginIndex if slotFound is true)
pair<int, int> firstFitAllocationMainSlotMatrix(set<pair<int, int>> lightTree, int partition, int requiredSlots)
{

    int firstIndex, lastIndex;
    if (partition == 0)
    {
        firstIndex = 0;
        lastIndex = SLOTS / 2 - 1;
    }
    else
    {
        firstIndex = SLOTS / 2;
        lastIndex = SLOTS - 1;
    }

    int beginIndex = 0;
    bool slotFound;
    for (int i = firstIndex; i <= lastIndex - requiredSlots + 1; i++)
    {
        slotFound = true;
        for (auto it : lightTree)
        {
            int u = it.first;
            int v = it.second;
            int slotIterator = i;
            int lastSlot = i + requiredSlots - 1;
            while (slotIterator <= lastSlot && slotMatrix[u][v].slots[slotIterator] == 0)
            {
                slotIterator++;
            }
            if (slotIterator == lastSlot + 1)
                continue;
            else
            {
                slotFound = false;
                break;
            }
        }
        if (slotFound == true)
        {
            beginIndex = i;
            break;
        }
    }

    if (slotFound == true)
    {
        //allocation
        for (auto it : lightTree)
        {
            int u = it.first;
            int v = it.second;
            int lastSlot = beginIndex + requiredSlots - 1;
            for (int i = beginIndex; i <= lastSlot; i++)
            {
                slotMatrix[u][v].slots[i] = 1;
            }
        }
    }

    if (slotFound == true)
    {
        return make_pair(1, beginIndex);
    }
    else
    {
        return make_pair(0, 0);
    }
}

//=========================ALLOCATION OF MAIN SLOT MATRIX=======================================================//



void cloneSlotMatrix(struct slotType (&slotMatrixClone)[VERTICES][VERTICES]){
    for (int i = 0; i < VERTICES; i++)
    {
        for (int j = 0; j < VERTICES; j++)
        {
            for (int k = 0; k < SLOTS; k++)
            {
                slotMatrixClone[i][j].slots[k] = slotMatrix[i][j].slots[k];
            }
        }
    }
}



int allocateSlots(set<pair<int, int>> lightTree, set<pair<int, int>> lightTreeBackup, int requiredSlots, int &beginIndexOriginal, int &beginIndexBackup, int req_id)
{


    pair<int,int> allocationStatusOriginal;
    pair<int,int> allocationStatusBackup;

    allocationStatusOriginal= firstFitAllocationMainSlotMatrix(lightTree,1,requiredSlots);
    allocationStatusBackup= firstFitAllocationMainSlotMatrix(lightTreeBackup,0,requiredSlots);

    int returnStatus=0;
    if(allocationStatusOriginal.first==0 && allocationStatusBackup.first ==0){
        returnStatus=0;
    }
    else if(allocationStatusOriginal.first==0 && allocationStatusBackup.first==1){
        deallocationMainSlotMatrix(lightTreeBackup,allocationStatusBackup.second,requiredSlots);
        returnStatus=0;
    }
    else if(allocationStatusOriginal.first==1 && allocationStatusBackup.first==0){
        deallocationMainSlotMatrix(lightTree,allocationStatusOriginal.second,requiredSlots);
        returnStatus=0;
    }
    else{
        returnStatus=1;
    }

    cout<<"Req Id is "<<req_id<<" Allocation Status is "<<allocationStatusOriginal.first<<" "<<allocationStatusBackup.first<<endl;
    beginIndexOriginal=allocationStatusOriginal.second;
    beginIndexBackup=allocationStatusBackup.second;
    return returnStatus;

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
int main(int argc,char **argv)
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


//===============================CREATE THE SEMAPHORE FOR PROCESS SYN==============================//
    
    const char *shm_name_id="id_shm";
    const char *shm_name_mat="mat_shm";
    //===============SEMAPHORE FOR PROTECTING REQ ID VALUE======================//
    if ((id_shm = shm_open(shm_name_id, O_RDWR | O_CREAT, S_IRWXU))<0) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(id_shm, sizeof(sem_t)) < 0 ) {
        perror("ftruncate");
        exit(1);
    }

    if ((id_semaphore = (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, id_shm, 0)) == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    //==============SEMAPHORE FOR PROTECTING SLOT MATRIX========================//
    if ((mat_shm = shm_open(shm_name_mat, O_RDWR | O_CREAT, S_IRWXU))<0) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(mat_shm, sizeof(sem_t)) < 0 ) {
        perror("ftruncate");
        exit(1);
    }

    if (( matrix_semaphore= (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, mat_shm, 0)) == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

//============================CREATION OF EMAPHORE FOR PROCESS SYN IS DONE=======================//

//============================INITITIALIZE CREATED SEMAPHORES====================================//
    sem_init(id_semaphore, 1, 1); 
    sem_init(matrix_semaphore,1, 1);
//============================INITITIALIZE CREATED SEMAPHORES====================================//


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
    
    // stores the FI
    float shmid6 = shmget(IPC_PRIVATE, totalRequests * sizeof(float), 0777 | IPC_CREAT);
    FI_array = (float *)shmat(shmid6, 0, 0);

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

    for (int i = 0; i < totalRequests; i++)
    {

        
        sem_wait(id_semaphore);
        if(number[0]<=totalRequests)
            fork();
        else
            break;
        
        if(number[0]<totalRequests)
            cout<<endl<<"Entered ID Critical Section"<<endl;

        int id = number[0];
        number[0]++;

        if(number[0]<totalRequests)
        {
            cout << "Request ID " << id << endl;
            cout << "Coming Out Of ID Critical Section" << endl<< endl;
        }
        //sem_post(id_semaphore);
        
        if (id == totalRequests) //on the last request, display BP and BBP
        {
            sleep(15);
            cout<<"id == totalReq waiting for all satisfied req to complete"<<endl;
            int sleepFlag = 1;
            while (sleepFlag == 1)
            {
                sleep(3);
                sem_wait(id_semaphore);
                if (number[1] >= totalRequests)
                    sleepFlag = 0;
                //sem_post(id_semaphore);
            }

            int noOfBlocked = 0;
            float total_FI = 0;
            float totalSlotsSatisfied = 0;
            float hopCountPrimary = 0;
            float hopCountBackup = 0;
            int c = 0;
            for (int i = 0; i < id; i++)
            {
                cout << "id: " << i << " status: " << requestsInfo[i][NO_OF_DEST + 1] << " " << FI_array[i] << endl;

                if (requestsInfo[i][NO_OF_DEST + 1] == 2)
                    noOfBlocked++;
                else if (requestsInfo[i][NO_OF_DEST + 1] == 0)
                {
                    // cout<<"id: "<<i<<" status: "<<requestsInfo[i][NO_OF_DEST + 1]<<" "<< FI_array[i]<<endl;
                    total_FI = total_FI + FI_array[i];
                    totalSlotsSatisfied = totalSlotsSatisfied + requestsInfo[i][NO_OF_DEST + 4];
                    hopCountPrimary += requestsInfo[i][NO_OF_DEST + 8];
                    hopCountBackup += requestsInfo[i][NO_OF_DEST + 9];
                    c++;
                }
            }
            float BP = (float)noOfBlocked / totalRequests;
            cout << id << " Blocked requests = " << noOfBlocked << ", BP = " << BP << endl;
            float BBP = (float)simulation_info[3] / (simulation_info[2] + simulation_info[3]);
            cout << id << "   "
                 << "BBP = " << BBP << endl;
            cout << "is " << ((totalRequests - noOfBlocked)) << " c is " << c << endl;
            float averageFI = total_FI / (totalRequests - noOfBlocked);
            cout << id << "   "
                 << "Average FI of satisfied req = " << averageFI << endl;
            float averageSlotUtilisationPrimary = 0;
            float averageSlotUtilisationBackup = 0;
            float networkLoad = totalRequests;
            float averageHopCountPrimary = hopCountPrimary / (totalRequests - noOfBlocked);
            float averageHopCountBackup = hopCountBackup / (totalRequests - noOfBlocked);
            float averageSlotCount = totalSlotsSatisfied / (totalRequests - noOfBlocked);
            averageSlotUtilisationPrimary = (networkLoad * averageHopCountPrimary * averageSlotCount) / (EDGES * SLOTS);
            averageSlotUtilisationBackup = (networkLoad * averageHopCountBackup * averageSlotCount) / (EDGES * SLOTS);
            cout << id << "   "
                 << "Average Slot Utilisation Primary = " << averageSlotUtilisationPrimary<< endl;            
            cout << id << "   "
                 << "Average Slot Utilisation Backup = "  <<  averageSlotUtilisationBackup<< endl;            
            cout << "Filename " << argv[0] << endl;
            /*
            ofstream myfile;
            myfile.open("dataset1.txt", ios_base::app);
            myfile << VERTICES << "," << EDGES << "," << totalRequests << "," << NO_OF_DEST << "," << SLOTS << "," << BP << "," << BBP << endl;
            myfile.close();
            */

            //======================PRINT THE SLOTMATRICES TO FILE==================//
            // writeToFile();
        }

        if (id >= totalRequests) //when we have generated 600 requests, terminate calling process
        {
            cout<<"id is "<<id<<" last code "<<totalRequests<<endl;
            sem_wait(id_semaphore);
            int completed=number[1];
            //sem_post(id_semaphore);
            // int sleepFlag_2=1;
            // while(sleepFlag_2==1){
            //     sleep(3);
            //     sem_wait(id_semaphore);
            //     cout<<"number[1] is "<<number[1]<<endl;
            //     if(number[1]>=totalRequests)
            //         number[1]++;
            //         sleepFlag_2=0;
            //     //sem_post(id_semaphore);
            // } 
            if(completed==totalRequests){
                number[1]++;
                shmdt(slotMatrix);
                shmctl(shmid, IPC_RMID, NULL);

                shmdt(simulation_info);
                shmctl(shmid1, IPC_RMID, NULL);

                shmdt(requestsInfo);
                shmctl(shmid4, IPC_RMID, NULL);

                shmdt(number);
                shmctl(shmid5, IPC_RMID, NULL);

                shmdt(FI_array);
                shmctl(shmid6, IPC_RMID, NULL);

                shm_unlink(shm_name_id);
                sem_destroy(id_semaphore);

                shm_unlink(shm_name_mat);
                sem_destroy(matrix_semaphore);
            }  

            
            // kill(getpid(), SIGKILL);
            // cout<<"SHARED_MEMORY DESTROYED"<<endl
            exit(2);

        }

        // -----when id < totalRequests, Common to every request-----------------------
        pid = getpid();
        unsigned tt = unsigned(pid) + unsigned(time(0));
        // srand(tt); //seeds random
        // srand(1); //seeds random
        srand(((id)%randomFactor) + 1); //seeds random
        int burstTime = 0;
        int src = 0;

        int noOfSlotsReq = generateRequest(burstTime, src, id); //burstTime and src is passed by ref, gets updated and send
        
        //PRINT REQ INFO
        cout << "Reqq id: " << id << " Src: " << src;
        cout<< " Dst: {";
        for(int i=0;i<NO_OF_DEST;i++){
            cout<< requestsInfo[id][i+1] << ",";
        }
        cout << " } Slots Req: " << noOfSlotsReq <<" Burst Time: "<<burstTime <<endl;
        
        int destinations[NO_OF_DEST];
        for(int i=0;i<NO_OF_DEST;i++){
            destinations[i]=requestsInfo[id][i+1];
        }

        set<pair<int, int>> lightTree = dijkstra(src, destinations);
        set<pair<int, int>> lightTreeBackup = secondary_dijkstra(src, destinations, lightTree);

        // cout<<"orig light tree"<<endl;
        // for(auto it: lightTree) {
        //     cout<<"("<<it.first<<","<<it.second<<") ";
        // }
        // cout<<endl;
        // cout<<"bakcup light tree"<<endl;
        // for(auto it: lightTreeBackup) {
        //     cout<<"("<<it.first<<","<<it.second<<") ";
        // }
        // cout<<endl;

        sort(id); // sort the destinations in requests according to their value
        int beginIndexOriginal = 0;
        int beginIndexBackup = 0;
        int isSlotsAllocated=0;
        enum blocking reasonOfBlocking;

        if(lightTree.size()==0 || lightTreeBackup.size()==0){

            if(lightTree.size()==0)
                cout<<"LIGHT TREE NOT FOUND FOR REQ ID "<<id<<" !!!!"<<endl;
            if(lightTreeBackup.size()==0)
                cout<<"BACKUP LIGHT TREE NOT FOUND FOR REQ ID "<<id<<" !!!!"<<endl;
            
            reasonOfBlocking=lightTreeNotFound;
            isSlotsAllocated=0;

        }
        else{
            sem_wait(matrix_semaphore);

            cout<<"Inside Allocation Critical Section Of Req ID "<<id<<endl;
            isSlotsAllocated = allocateSlots(lightTree, lightTreeBackup, noOfSlotsReq + 2, beginIndexOriginal, beginIndexBackup,id);
            cout<<"Coming Out Of Allocation Critical Section Of Req ID "<<id<<endl;

            cout<<"Inside Slot Matrix State Critical Section Of Req ID "<<id<<endl;
            for (int i = 0; i < VERTICES; i++)
            {
                for (int j = 0; j < VERTICES; j++)
                {
                    for (int k = 0; k < SLOTS; k++)
                    {
                        slotMatrixStates[id][i][j].slots[k] = slotMatrix[i][j].slots[k];
                    }
                }
            }

            // printSlotMatrixState(id);
            // printMainSlotMatrix();
            if(isSlotsAllocated==1){
                FI_array[id]=findFIMainSlotMatrix(lightTree)+findFIMainSlotMatrix(lightTreeBackup);
            }

            //sem_post(matrix_semaphore);

            if(isSlotsAllocated==0)
                reasonOfBlocking = UnableToAllocateSlots;
        }


            //store the state of slotmatrix in memory<<endl;
            // sem_wait(matrix_semaphore);
            
            // cout<<"Inside Slot Matrix State Critical Section Of Req ID "<<id<<endl;
            // for (int i = 0; i < VERTICES; i++)
            // {
            //     for (int j = 0; j < VERTICES; j++)
            //     {
            //         for (int k = 0; k < SLOTS; k++)
            //         {
            //             slotMatrixStates[id][i][j].slots[k] = slotMatrix[i][j].slots[k];
            //         }
            //     }
            // }
            // cout<<"Coming Out Of Slot Matrix State Critical Section Of Req ID "<<id<<endl;

            //sem_post(matrix_semaphore);


        if (isSlotsAllocated == 1) // SLOTS successfully allocated for the request
        {

            requestsInfo[id][NO_OF_DEST + 1] = 1; // 1 means request is active
            requestsInfo[id][NO_OF_DEST + 2] = beginIndexOriginal;
            requestsInfo[id][NO_OF_DEST + 3] = beginIndexBackup;
            requestsInfo[id][NO_OF_DEST + 4] = noOfSlotsReq;
            requestsInfo[id][NO_OF_DEST + 5] = burstTime;
            requestsInfo[id][NO_OF_DEST + 6] = id;
            requestsInfo[id][NO_OF_DEST + 7] = INT_MIN;
            requestsInfo[id][NO_OF_DEST + 8] = lightTree.size();       //hop count primary
            requestsInfo[id][NO_OF_DEST + 9] = lightTreeBackup.size(); //hop count backup

            sleep(burstTime);
            requestsInfo[id][NO_OF_DEST + 1] = 0;   // 0 means completed
            simulation_info[2] += noOfSlotsReq + 2; // simulation_info[2] : no of SLOTS allocated
            
            //SLOTS DEALLOCATION

            // deallocation(slotMatrix,lightTree,beginIndexOriginal,noOfSlotsReq+2);
            // deallocation(slotMatrix,lightTreeBackup,beginIndexBackup,noOfSlotsReq+2);


            //===============LOCK WHILE A PROCESS IS DEALLOCATING MATRIX=================//
            sem_wait(matrix_semaphore);
            
            deallocationMainSlotMatrix(lightTree,beginIndexOriginal,noOfSlotsReq+2);
            deallocationMainSlotMatrix(lightTreeBackup,beginIndexBackup,noOfSlotsReq+2);
            reqSatisfied[id]=1;
            simulation_info[0]++; // simulation_info[0] : no of requests completed
            cout<<"REQUEST "<<id<<" SATISFIED"<<"\n\n";
            
            //sem_post(matrix_semaphore);
            //===============LOCK WHILE A PROCESS IS DEALLOCATING MATRIX=================//


            int k = rand() % 3;
            sleep(k); // after this request, process sleeps for a random amount of time before spawning another process/request
        }
        else //no SLOTS available, request has been blocked
        {
            //requestsInfo[id][4] = 2;            //2 means blocked
            requestsInfo[id][NO_OF_DEST + 1] = 2;
            requestsInfo[id][NO_OF_DEST + 2] = INT_MIN; //begin slot orignal
            requestsInfo[id][NO_OF_DEST + 3] = INT_MIN; //begin slot backup
            requestsInfo[id][NO_OF_DEST + 4] = noOfSlotsReq; //requires slot
            requestsInfo[id][NO_OF_DEST + 5] = burstTime; //burst time
            requestsInfo[id][NO_OF_DEST + 6] = id; //id
            requestsInfo[id][NO_OF_DEST + 7] = reasonOfBlocking; //reasonOfBlocking
            requestsInfo[id][NO_OF_DEST + 8] = lightTree.size();       //hop count primary
            requestsInfo[id][NO_OF_DEST + 9] = lightTreeBackup.size(); //hop count backup

            reqSatisfied[id] =0;
            simulation_info[3] += noOfSlotsReq; // simulation_info[3] : no of SLOTS blocked
            simulation_info[1]++;               // simulation_info[1] : no of requests blocked
            //requestsInfo[id][7] = id;
            cout <<"REQUEST "<<id<<" BLOCKED\n\n";
            sleep(3); // WHY are we sleeping here after request is getting blocked???
        }
        /*     cout << id << " source: " << requestsInfo[id][0] << " dest: (" << requestsInfo[id][1] << "," << requestsInfo[id][2] << "," << requestsInfo[id][3] << ") status: "
         << requestsInfo[id][4] << " beg slot: " << requestsInfo[id][5] << " no of SLOTS: " << requestsInfo[id][6] << " no of requets: " << requestsInfo[id][7] << endl;
    */
        /*for (int j = 0; j < 8; j++)
      cout << requestsInfo[id][j] << " ";
    cout << endl;
    */
        number[1]++;    
    }

    while (true)
    {
        // cout<<"ASdasdasd";
        int status;
        pid_t done = wait(&status);
        if (done == -1)
        {
            if (errno == ECHILD)
            {
                break; // no more child processes
            }
        }
        else
        {
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            {
                // cerr << "pid " << done << " failed" << endl;
                exit(1);
            }
        }
    }
    

    return 0;
}
