"""
Graph to find dijsktra's shortest path and light tree
"""

import sys
from collections import defaultdict
from heap import Heap


class Graph():
    """ Graph to find dijsktra's shortest path and light tree"""

    def __init__(self, num_of_vertices, inputFile):
        self.V = num_of_vertices
        self.graph = defaultdict(list)
        self._initialize_graph(inputFile)
        self.curr_src = 0
        self.shortest_dist = []  # shortest distance to all the vertices from the current src

    def _initialize_graph(self, file):
        '''initializes the network graph from txt file'''
        f = open(file, "r")
        contents = f.readlines()
        for line in contents:
            values = [int(x) for x in line[:-1].split(" ")]
            self.addEdge(values[0], values[1], values[2])

    def addEdge(self, src, dest, weight):
        ''' Adds an edge to an undirected graph'''
        self.graph[src].insert(0, [dest, weight])
        # Since graph is undirected
        self.graph[dest].insert(0, [src, weight])

    def dijkstra(self, src):
        '''Calculates the shortest path distance from src to all other vertices
            and stores them in self.shortest_dist list.'''
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
        if not self.curr_src == src:
            self.curr_src = src
            self.shortest_dist = self.dijkstra(src)
        path = []
        path.append(dest)
        dist = self.shortest_dist
        graph = self.graph
        curr = dest
        while not curr == src:

            for child in graph[curr]:
                # print("child for {}".format(curr), child)
                if dist[curr] - child[1] == dist[child[0]]:
                    curr = child[0]
                    path.insert(0, curr)
        return path
