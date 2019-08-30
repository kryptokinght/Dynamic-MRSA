from collections import defaultdict
import sys


class Heap():

    def __init__(self):
        self.array = []
        self.size = 0
        self.pos = []

    def newMinHeapNode(self, v, dist):
        minHeapNode = [v, dist]
        return minHeapNode

    # A utility function to swap two nodes of min heap. Needed for min heapify
    def swapMinHeapNode(self, a, b):
        t = self.array[a]
        self.array[a] = self.array[b]
        self.array[b] = t

    # A standard function to heapify at given index
    # This function also updates position of nodes
    # when they are swapped.Position is needed
    # for decreaseKey()
    def minHeapify(self, idx):
        smallest = idx
        left = 2*idx + 1
        right = 2*idx + 2

        if left < self.size and self.array[left][1] \
                < self.array[smallest][1]:
            smallest = left

        if right < self.size and self.array[right][1]\
                < self.array[smallest][1]:
            smallest = right

        # The nodes to be swapped in min
        # heap if idx is not smallest
        if smallest != idx:

            # Swap positions
            self.pos[self.array[smallest][0]] = idx
            self.pos[self.array[idx][0]] = smallest

            # Swap nodes
            self.swapMinHeapNode(smallest, idx)

            self.minHeapify(smallest)

    # Standard function to extract minimum
    # node from heap
    def extractMin(self):

        # Return NULL wif heap is empty
        if self.isEmpty() == True:
            return

        # Store the root node
        root = self.array[0]

        # Replace root node with last node
        lastNode = self.array[self.size - 1]
        self.array[0] = lastNode

        # Update position of last node
        self.pos[lastNode[0]] = 0
        self.pos[root[0]] = self.size - 1

        # Reduce heap size and heapify root
        self.size -= 1
        self.minHeapify(0)

        return root

    def isEmpty(self):
        return True if self.size == 0 else False

    def decreaseKey(self, v, dist):

        # Get the index of v in heap array

        i = self.pos[v]

        # Get the node and update its dist value
        self.array[i][1] = dist

        # Travel up while the complete tree is
        # not hepified. This is a O(Logn) loop
        while i > 0 and self.array[i][1] < self.array[(i - 1) / 2][1]:

            # Swap this node with its parent
            self.pos[self.array[i][0]] = (i-1)/2
            self.pos[self.array[(i-1)/2][0]] = i
            self.swapMinHeapNode(i, (i - 1)/2)

            # move to parent index
            i = (i - 1) / 2

    # A utility function to check if a given
    # vertex 'v' is in min heap or not
    def isInMinHeap(self, v):

        if self.pos[v] < self.size:
            return True
        return False


class Graph():

    def __init__(self, V):
        self.V = V
        self.graph = defaultdict(list)
        self._initializeGraph('network_graph.txt')
        self.currSrc = 0
        self.shortestDist = []  # shortest distance to all the vertices from the current src

    def _initializeGraph(self, file):
        '''initializes the network graph from txt file'''
        f = open("network_graph.txt", "r")
        contents = f.readlines()
        for line in contents:
            values = [int(x) for x in line[:-1].split(" ")]
            self.addEdge(values[0], values[1], values[2])

    def addEdge(self, src, dest, weight):
        ''' Adds an edge to an undirected graph'''
        self.graph[src].insert(0, [dest, weight])
        self.graph[dest].insert(0, [src, weight])  # Since graph is undirected

    def dijkstra(self, src):
        V = self.V  # number of vertices in graph
        dist = []  # dist values used to pick minimum weight edge in cut

        # minHeap represents set E
        minHeap = Heap()

        # Initialize min heap with all vertices.
        # dist value of all vertices
        for v in range(V):
            dist.append(sys.maxint)
            minHeap.array.append(minHeap.newMinHeapNode(v, dist[v]))
            minHeap.pos.append(v)

        # Make dist value of src vertex as 0 so
        # that it is extracted first
        minHeap.pos[src] = src
        dist[src] = 0
        minHeap.decreaseKey(src, dist[src])

        # Initially size of min heap is equal to V
        minHeap.size = V

        # In the following loop, min heap contains all nodes
        # whose shortest distance is not yet finalized.
        while minHeap.isEmpty() == False:

            # Extract the vertex with minimum distance value
            newHeapNode = minHeap.extractMin()
            u = newHeapNode[0]

            # Traverse through all adjacent vertices of
            # u (the extracted vertex) and update their
            # distance values
            for pCrawl in self.graph[u]:

                v = pCrawl[0]

                # If shortest distance to v is not finalized
                # yet, and distance to v through u is less
                # than its previously calculated distance
                if minHeap.isInMinHeap(v) and dist[u] != sys.maxint and \
                        pCrawl[1] + dist[u] < dist[v]:
                    dist[v] = pCrawl[1] + dist[u]

                    # update distance value
                    # in min heap also
                    minHeap.decreaseKey(v, dist[v])
        # printArr(dist, V)
        return dist

    def getShortestPath(self, src, dest):
        '''returns the shortest path as a list of vertices from src to dest'''
        if not self.currSrc == src:
            self.currSrc = src
            self.shortestDist = self.dijkstra(src)
        path = []
        path.append(dest)
        dist = self.shortestDist
        graph = self.graph
        curr = dest
        while not curr == src:

            for child in graph[curr]:
                #print("child for {}".format(curr), child)
                if dist[curr] - child[1] == dist[child[0]]:
                    curr = child[0]
                    path.insert(0, curr)
        return path


# Driver program to test the above functions
graph = Graph(9)
#graph.shortestDist = graph.dijkstra(0)

path = graph.getShortestPath(8, 4)
print(path)
