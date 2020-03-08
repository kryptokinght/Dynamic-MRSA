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

using namespace std;

//VARIABLES
#define T 4 //not used
#define noOfRequests 10
#define totalRequests 10
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
// int Result[VERTICES][VERTICES];     // Stores the edges of the light tree
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

int generateRequest(int *bt, int *src, int requestId)
{
    /* Generates a request and returns the number of SLOTS for the request */
    for (int i = 0; i < VERTICES; i++)
        activePoint[i] = 0;         //
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
    cout << "ORIGINAL PARENT" << endl;
    for (int i = 0; i < VERTICES; i++)
    {
        cout << i << " :" << parent[i] << ", ";
    }
    cout << endl;
    set<pair<int, int>> s = getLightTree(parent, src, destinations); //adds to result
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

    cout << "BACKUP PARENT" << endl;
    for (int i = 0; i < VERTICES; i++)
    {
        cout << i << " :" << parent[i] << ", ";
    }
    cout << endl;
    set<pair<int, int>> s = getLightTree(parent, src, destinations); //adds to result
    return s;
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


void deallocation(struct slotType (&slotMatrixClone)[VERTICES][VERTICES], set<pair<int, int>> lightTree, int beginIndex, int requiredSlots){
    // deallocation
    for (auto it : lightTree)
    {
        int u = it.first;
        int v = it.second;
        int lastSlot = beginIndex + requiredSlots - 1;
        for (int i = beginIndex; i <= lastSlot; i++)
        {
            slotMatrixClone[u][v].slots[i] = 0;
        }
    }
}

float findFI(struct slotType (&slotMatrixClone)[VERTICES][VERTICES], set<pair<int, int>> lightTree)
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
            while (k < SLOTS && slotMatrixClone[u][v].slots[k] == 0)
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


//returns a pair of (slotFound and beginIndex if slotFound is true)
pair<int, int> firstFitAllocation(struct slotType (&slotMatrixClone)[VERTICES][VERTICES], set<pair<int, int>> lightTree, int partition, int requiredSlots)
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
    bool slotFound = false;
    for (int i = firstIndex; i <= lastIndex - requiredSlots + 1; i++)
    {
        slotFound = true;
        for (auto it : lightTree)
        {
            int u = it.first;
            int v = it.second;
            int slotIterator = i;
            int lastSlot = i + requiredSlots - 1;
            while (slotIterator <= lastSlot && slotMatrixClone[u][v].slots[slotIterator] == 0)
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
                slotMatrixClone[u][v].slots[i] = 1;
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

int  findBestPolicy(set<pair<int, int>> lightTree, set<pair<int, int>> lightTreeBackup, int requiredSlots)
{
    //cloning the shared memroy slots
    slotType slotMatrixClone[VERTICES][VERTICES];
    cloneSlotMatrix(slotMatrixClone);

    pair<int,int> allocationStatusOriginal;
    pair<int,int> allocationStatusBackup;
    bool is_blocked_policy_A=false;
    bool is_blocked_policy_B=false;
    bool is_blocked_policy_C=false;
    float FI_policy_A=0;
    float FI_policy_B=0;
    float FI_policy_C=0;
    int min_last_index_A=0;
    int min_last_index_B=0;
    int min_last_index_C=0;

    //policy A (Original in Primary partition=0 and Backup in Secondary partition=1)
    allocationStatusOriginal= firstFitAllocation(slotMatrixClone,lightTree,0,requiredSlots);
    allocationStatusBackup= firstFitAllocation(slotMatrixClone,lightTreeBackup,1,requiredSlots);
    
    if(allocationStatusOriginal.first==0 && allocationStatusBackup.first==0){
        is_blocked_policy_A=true;
        is_blocked_policy_B=true;
        is_blocked_policy_C=true;
    }
    else if(allocationStatusOriginal.first==1 && allocationStatusBackup.first==0){
        is_blocked_policy_A=true;
        is_blocked_policy_C=true;

        //==========================go for policy B(Original in Primary partition=0 and Backup in Primary partition=0)=================
        allocationStatusBackup = firstFitAllocation(slotMatrixClone,lightTreeBackup,0,requiredSlots);

        if(allocationStatusBackup.first==0)
            is_blocked_policy_B=true;
        else{
            //both paths satisfied so finding the FI
            FI_policy_B = findFI(slotMatrixClone,lightTree) + findFI(slotMatrixClone,lightTreeBackup);
            min_last_index_B=max(allocationStatusOriginal.second+requiredSlots-1,allocationStatusBackup.second+requiredSlots-1);
            deallocation(slotMatrixClone,lightTree,allocationStatusOriginal.second,requiredSlots);
            deallocation(slotMatrixClone,lightTreeBackup,allocationStatusBackup.second,requiredSlots);
        }

    }
    else if(allocationStatusOriginal.first==0 && allocationStatusBackup.first==1){
        is_blocked_policy_A=true;
        is_blocked_policy_B=true;

        //=========================go for policy C(Original in Secondary partition=1 and Backup in Secondary partition=1)====================
        allocationStatusOriginal = firstFitAllocation(slotMatrixClone,lightTree,1,requiredSlots);
        if(allocationStatusOriginal.first==0)
            is_blocked_policy_C=true;
        else{
            //both paths satisfied so finding the FI
            FI_policy_C = findFI(slotMatrixClone,lightTree) + findFI(slotMatrixClone,lightTreeBackup);
            min_last_index_C=max(allocationStatusOriginal.second+requiredSlots-1,allocationStatusBackup.second+requiredSlots-1);            
            deallocation(slotMatrixClone,lightTree,allocationStatusOriginal.second,requiredSlots);
            deallocation(slotMatrixClone,lightTreeBackup,allocationStatusBackup.second,requiredSlots);
        }

    }
    else{
        //find FI for all the policies

        //========================go for policy A(Original in Primary partition=0 and Backup in Primary partition=1)=================//

        //both paths satisfied so finding the FI
        FI_policy_A = findFI(slotMatrixClone,lightTree) + findFI(slotMatrixClone,lightTreeBackup);
        min_last_index_A=max(allocationStatusOriginal.second+requiredSlots-1,allocationStatusBackup.second+requiredSlots-1);      
        //deallocate backup path in secondary part for policy B
        deallocation(slotMatrixClone,lightTreeBackup,allocationStatusBackup.second,requiredSlots);
        

        //========================go for policy B(Original in Primary partition=0 and Backup in Primary partition=0)============
        allocationStatusBackup = firstFitAllocation(slotMatrixClone,lightTreeBackup,0,requiredSlots);

        if(allocationStatusBackup.first==0)
            is_blocked_policy_B=true;
        else{
            //both paths satisfied so finding the FI
            FI_policy_B = findFI(slotMatrixClone,lightTree) + findFI(slotMatrixClone,lightTreeBackup);
            min_last_index_B=max(allocationStatusOriginal.second+requiredSlots-1,allocationStatusBackup.second+requiredSlots-1) ;           
            deallocation(slotMatrixClone,lightTreeBackup,allocationStatusBackup.second,requiredSlots);
        }
        deallocation(slotMatrixClone,lightTree,allocationStatusOriginal.second,requiredSlots);


        //========================go for policy C(Original in Secondary partition=1 and Backup in Secondary partition=1)====================
        allocationStatusOriginal = firstFitAllocation(slotMatrixClone,lightTree,1,requiredSlots);
        allocationStatusBackup = firstFitAllocation(slotMatrixClone,lightTreeBackup,1,requiredSlots);
        if(allocationStatusOriginal.first==0)
            is_blocked_policy_C=true;
        else{
            //both paths satisfied so finding the FI
            FI_policy_C = findFI(slotMatrixClone,lightTree) + findFI(slotMatrixClone,lightTreeBackup);
            min_last_index_C=max(allocationStatusOriginal.second+requiredSlots-1,allocationStatusBackup.second+requiredSlots-1);            
            deallocation(slotMatrixClone,lightTree,allocationStatusOriginal.second,requiredSlots);
            deallocation(slotMatrixClone,lightTreeBackup,allocationStatusBackup.second,requiredSlots);
        }
    }


    //=========================CHOOSE THE BEST FI AND RETURN=================================//
    //8 COMBINATIONS ARE THERE
    //all blocked
    if(is_blocked_policy_A==true && is_blocked_policy_B==true && is_blocked_policy_C==true){
        return 0;
    }
    else if(is_blocked_policy_A==true && is_blocked_policy_B==true && is_blocked_policy_C==false){
        //only C is there
        return 3;
    }
    else if(is_blocked_policy_A==true && is_blocked_policy_B==false && is_blocked_policy_C==true){
        //only B is there
        return 2;
    }
    else if(is_blocked_policy_A==true && is_blocked_policy_B==false && is_blocked_policy_C==false){
        //between B and C
        if(FI_policy_B>FI_policy_C)
            return 2;
        else if(FI_policy_B<FI_policy_C)
            return 3;
        else{
            if(min_last_index_B<=min_last_index_C)
                return 2;
            else
                return 3;
        }        
    }
    else if(is_blocked_policy_A==false && is_blocked_policy_B==true && is_blocked_policy_C==true){
        return 1;
    }
    else if(is_blocked_policy_A==false && is_blocked_policy_B==true && is_blocked_policy_C==false){
        //between A and C
        if(FI_policy_A>FI_policy_C)
            return 1;
        else if(FI_policy_A<FI_policy_C)
            return 3;
        else{
            if(min_last_index_A<=min_last_index_C)
                return 1;
            else
                return 3;
        }    
    }
    else if(is_blocked_policy_A==false && is_blocked_policy_B==false && is_blocked_policy_C==true){
        //between A and B
        if(FI_policy_A>FI_policy_B)
            return 1;
        else if(FI_policy_A<FI_policy_B)
            return 2;
        else{
            if(min_last_index_A<=min_last_index_B)
                return 1;
            else
                return 2;
        }   
    }
    else if(is_blocked_policy_A==false && is_blocked_policy_B==false && is_blocked_policy_C==false){
        
        //all available
        if(FI_policy_A>FI_policy_B){
            if(FI_policy_A>FI_policy_C)
                return 1;
            else if(FI_policy_A<FI_policy_C)
                return 3;
            else{
                if(min_last_index_A<=min_last_index_C)
                    return 1;
                else
                    return 3;
            }
        }
        else if(FI_policy_A<FI_policy_B){
            if(FI_policy_B>FI_policy_C)
                return 2;
            else if(FI_policy_B<FI_policy_C)
                return 3;
            else{
                if(min_last_index_B<=min_last_index_C)
                    return 2;
                else
                    return 3;
            }
        }
        else{
            if(FI_policy_A<FI_policy_C)
                return 3;
            else if(FI_policy_A>FI_policy_C){
                if(min_last_index_A<=min_last_index_B)
                    return 1;
                else
                    return 2;
            }
            else{
                if(min_last_index_A<=min_last_index_B && min_last_index_A<=min_last_index_C){
                    return 1;                   
                }
                else if(min_last_index_B<min_last_index_A && min_last_index_B<=min_last_index_C){
                    return 2;
                }
                else if(min_last_index_C<min_last_index_A && min_last_index_C<min_last_index_B){
                    return 3;
                }
            }
        }
    
    }

    //==============================================================================================//
        
}

int allocateSlots(set<pair<int, int>> lightTree, set<pair<int, int>> lightTreeBackup, int requiredSlots, int &beginIndexOriginal, int &beginIndexBackup, int req_id)
{

    int bestPolicy= findBestPolicy(lightTree,lightTreeBackup,requiredSlots);
    
    //blocked request
    if(bestPolicy=0)
        return 0;
    else{
        pair<int,int> allocationStatusOriginal;
        pair<int,int> allocationStatusBackup;

        if(bestPolicy==1){
            // allocationStatusOriginal= firstFitAllocation(slotMatrix,lightTree,0,requiredSlots);
            // allocationStatusBackup= firstFitAllocation(slotMatrix,lightTreeBackup,1,requiredSlots);
            allocationStatusOriginal= firstFitAllocationMainSlotMatrix(lightTree,0,requiredSlots);
            allocationStatusBackup= firstFitAllocationMainSlotMatrix(lightTreeBackup,1,requiredSlots);
        }
        else if(bestPolicy==2){
            // allocationStatusOriginal= firstFitAllocation(slotMatrix,lightTree,0,requiredSlots);
            // allocationStatusBackup= firstFitAllocation(slotMatrix,lightTreeBackup,0,requiredSlots);
            allocationStatusOriginal= firstFitAllocationMainSlotMatrix(lightTree,0,requiredSlots);
            allocationStatusBackup= firstFitAllocationMainSlotMatrix(lightTreeBackup,0,requiredSlots);
        }
        else if(bestPolicy==3){
            // allocationStatusOriginal= firstFitAllocation(slotMatrix,lightTree,1,requiredSlots);
            // allocationStatusBackup= firstFitAllocation(slotMatrix,lightTreeBackup,1,requiredSlots);
            allocationStatusOriginal= firstFitAllocationMainSlotMatrix(lightTree,1,requiredSlots);
            allocationStatusBackup= firstFitAllocationMainSlotMatrix(lightTreeBackup,1,requiredSlots);
        }
        beginIndexOriginal=allocationStatusOriginal.second;
        beginIndexBackup=allocationStatusBackup.second;
        return 1;
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
        
        sem_wait(id_semaphore);

        cout<<endl<<"ENTERED ID CRITICAL SECTION"<<endl;
        int id = number[0];
        number[0]++;
        cout<<"REQUEST ID "<<id<<endl;;
        cout<<"COMING OUT OF ID CRITICAL SECTION"<<endl<<endl;;
        sem_post(id_semaphore);
        
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

            //======================PRINT THE SLOTMATRICES TO FILE==================//
            ofstream myfile;
            string filenam = "samples/slot_matrices.txt";
            myfile.open(filenam, ios_base::out);
            //myfile << VERTICES << "," << EDGES << "," << totalRequests << "," << NO_OF_DEST << "," << SLOTS << "," << BP << "," << BBP << endl;
            //printing the slot

            for(int req_id=0; req_id<totalRequests;req_id++){
                myfile << "Reqq id: " << req_id << " Src: " << requestsInfo[req_id][0] << " Dst: {" << requestsInfo[req_id][1] << "," << requestsInfo[req_id][2] << "," << requestsInfo[req_id][3] << "} slots: " << requestsInfo[req_id][NO_OF_DEST+4] << endl;
                myfile<< "SLOT MATRIX \n";
                myfile<< "\t\t";
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
                            myfile << slotMatrix[i][j].slots[k] << "   ";
                        }
                        myfile << endl;
                    
                    }
                }
            }
            myfile.close();
            //end of slot print

        }

        if (id >= totalRequests) //when we have generated 600 requests, terminate calling process
        {
            // cout<<"id is "<<id<<" last code "<<totalRequests<<endl;
            shmdt(slotMatrix);
            shmctl(shmid, IPC_RMID, NULL);

            shmdt(simulation_info);
            shmctl(shmid1, IPC_RMID, NULL);

            shmdt(requestsInfo);
            shmctl(shmid4, IPC_RMID, NULL);

            shmdt(number);
            shmctl(shmid5, IPC_RMID, NULL);

            shm_unlink(shm_name_id);
            sem_destroy(id_semaphore);

            shm_unlink(shm_name_mat);
            sem_destroy(matrix_semaphore);

            kill(getpid(), SIGKILL);

        }

        // -----when id < totalRequests, Common to every request-----------------------
        pid = getpid();
        unsigned tt = unsigned(pid) + unsigned(time(0));
        srand(tt); //seeds random
        int burstTime = 0;
        int src = 0;

        int noOfSlotsReq = generateRequest(&burstTime, &src, id); //burstTime and src is passed by ref, gets updated and send
        cout << "Reqq id: " << id << " Src: " << src << " Dst: {" << requestsInfo[id][1] << "," << requestsInfo[id][2] << "," << requestsInfo[id][3] << "} slots: " << noOfSlotsReq << endl;
        int destinations[] = {requestsInfo[id][1], requestsInfo[id][2], requestsInfo[id][3]};
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
        if(lightTree.size()==0 || lightTreeBackup.size()==0){
            cout<<"LIGHT TREE NOT FOUND !!!!"<<endl;
            isSlotsAllocated=0;
        }
        else{
            sem_wait(matrix_semaphore);
            isSlotsAllocated = allocateSlots(lightTree, lightTreeBackup, noOfSlotsReq + 2, beginIndexOriginal, beginIndexBackup,id);
            sem_post(matrix_semaphore);
        }


            //store the state of slotmatrix in memory<<endl;
            sem_wait(matrix_semaphore);
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
            sem_post(matrix_semaphore);


        if (isSlotsAllocated == 1) // SLOTS successfully allocated for the request
        {

            requestsInfo[id][NO_OF_DEST + 1] = 1; // 1 means request is active
            requestsInfo[id][NO_OF_DEST + 2] = beginIndexOriginal;
            requestsInfo[id][NO_OF_DEST + 3] = beginIndexBackup;
            requestsInfo[id][NO_OF_DEST + 4] = noOfSlotsReq;
            requestsInfo[id][NO_OF_DEST + 5] = id;
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
            sem_post(matrix_semaphore);
            //===============LOCK WHILE A PROCESS IS DEALLOCATING MATRIX=================//


            int k = rand() % 3;
            sleep(k); // after this request, process sleeps for a random amount of time before spawning another process/request
        }
        else //no SLOTS available, request has been blocked
        {
            //requestsInfo[id][4] = 2;            //2 means blocked
            requestsInfo[id][NO_OF_DEST + 1] = 2;
            requestsInfo[id][NO_OF_DEST + 4] = noOfSlotsReq;
            reqSatisfied[id] =0;
            simulation_info[3] += noOfSlotsReq; // simulation_info[3] : no of SLOTS blocked
            simulation_info[1]++;               // simulation_info[1] : no of requests blocked
            //requestsInfo[id][7] = id;
            requestsInfo[id][NO_OF_DEST + 4] = id;
            cout <<"Request "<<id<<" Blocked\n\n";
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
