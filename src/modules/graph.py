"""
Graph to find dijsktra's shortest path and light tree
"""
import sys
import random
import networkx as nx
import matplotlib.pyplot as plt


class Graph():
    """ Graph to find dijsktra's shortest path and light tree"""

    def __init__(self, num_of_vertices, filename):

        self.V = num_of_vertices
        #self.nx_graph = self._initialize_graph_random()
        self.graph = self._initialize_graph(filename)

    def _initialize_graph_random(self):
        '''Creates a graph using networkx and initializes self.nx_graph'''
        G = nx.erdos_renyi_graph(self.V, 0.3)
        for (u, v) in G.edges():
            wt = random.randint(1, 10)
            G.edges[u, v]['weight'] = wt
        return G

    def _initialize_graph(self, filename):
        '''Creates a graph from given file and initializes self.graph'''
        G = [[0 for i in range(self.V)] for i in range(self.V)]
        f = open(filename, 'r')
        contents = f.readlines()
        for line in contents:
            val = line.split(' ')
            val[2] = val[2][:-1]
            x = [int(d) for d in val]
            G[x[0]][x[1]] = x[2]
            G[x[1]][x[0]] = x[2]

        return G

    def minDistance(self, dist, sptSet):
        min = sys.maxsize
        for v in range(self.V):
            if dist[v] < min and sptSet[v] == False:
                min = dist[v]
                min_index = v
        return min_index

    def dijkstra(self, src, dest):

        dist = [sys.maxsize] * self.V
        dist[src] = 0
        sptSet = [False] * self.V

        for cout in range(self.V):
            u = self.minDistance(dist, sptSet)
            sptSet[u] = True

            for v in range(self.V):
                if self.graph[u][v] > 0 and sptSet[v] == False and \
                        dist[v] > dist[u] + self.graph[u][v]:
                    dist[v] = dist[u] + self.graph[u][v]

        path = []
        curr = dest
        path.append(curr)
        smallest = curr
        smallest_val = sys.maxsize
        while not curr == src:
            for i in range(self.V):
                if not self.graph[curr][i] == 0:
                    if dist[i] < smallest_val:
                        smallest = i
                        smallest_val = dist[i]
            curr = smallest
            path.insert(0, curr)
            smallest_val = sys.maxsize
        return path

    def getAllEdges(self):
        edges = []
        for i in range(self.V):
            for j in range(self.V):
                if not self.graph[i][j] == 0:
                    edges.append((i, j))
        return edges

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

    def getShortestPathRandom(self, src, dest):
        '''returns the shortest path as a list of vertices from src to dest'''
        paths = []
        for d in dest:
            path = nx.dijkstra_path(self.nx_graph, src, d, "weight")
            paths.append(path)
        return paths

    def getShortestPath(self, src, dest):
        '''returns the shortest path as a list of vertices from src to dest'''
        paths = []
        for d in dest:
            path = self.dijkstra(src, d)
            paths.append(path)
        return paths
