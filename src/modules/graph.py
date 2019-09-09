"""
Graph to find dijsktra's shortest path and light tree
"""
import sys
import math
import random
from collections import defaultdict
import networkx as nx
import matplotlib.pyplot as plt
from modules.heap import Heap


class Graph():
    """ Graph to find dijsktra's shortest path and light tree"""

    def __init__(self, num_of_vertices):
        self.V = num_of_vertices
        self.graph = defaultdict(list)
        self.nx_graph = self._initialize_graph()
        self.curr_src = 0
        self.shortest_dist = []  # shortest distance to all the vertices from the current src

    def _initialize_graph(self):
        '''Creates a graph using networkx and initializes self.graph'''
        G = nx.erdos_renyi_graph(self.V, 0.5)
        for (u, v) in G.edges():
            wt = random.randint(1, 10)
            G.edges[u, v]['weight'] = wt
            self.addEdge(u, v, wt)

        return G

    def show_graph(self):
        '''Displays the graph using mathplotlib.pyplot'''
        G = self.nx_graph
        pos = nx.spring_layout(G)
        nx.draw(G, pos)
        labels = nx.get_edge_attributes(G, 'weight')
        nx.draw_networkx_edge_labels(
            G, pos, edge_labels=labels, alpha=0.5, edge_color='b')
        # labels
        nx.draw_networkx_labels(G, pos, font_size=10, font_family='sans-serif')
        plt.show()

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
            dist.append(math.inf)
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
                if minHeap.isInMinHeap(v) and dist[u] != math.inf and \
                        pCrawl[1] + dist[u] < dist[v]:
                    dist[v] = pCrawl[1] + dist[u]

                    # update distance value
                    # in min heap also
                    minHeap.decreaseKey(v, dist[v])
        # printArr(dist, V)
        return dist

    def getShortestPath(self, src, dest):
        '''returns the shortest path as a list of vertices from src to dest'''
        ans = nx.dijkstra_path(self.nx_graph, src, dest, "weight")
        print(ans)
        return ans
        """ if not self.curr_src == src:
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
        return path """
