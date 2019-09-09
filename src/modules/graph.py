"""
Graph to find dijsktra's shortest path and light tree
"""
import random
import networkx as nx
import matplotlib.pyplot as plt


class Graph():
    """ Graph to find dijsktra's shortest path and light tree"""

    def __init__(self, num_of_vertices):
        self.V = num_of_vertices
        self.nx_graph = self._initialize_graph()

    def _initialize_graph(self):
        '''Creates a graph using networkx and initializes self.graph'''
        G = nx.erdos_renyi_graph(self.V, 0.3)
        for (u, v) in G.edges():
            wt = random.randint(1, 10)
            G.edges[u, v]['weight'] = wt
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

    def getShortestPath(self, src, dest):
        '''returns the shortest path as a list of vertices from src to dest'''
        ans = nx.dijkstra_path(self.nx_graph, src, dest, "weight")
        return ans
