import networkx as nx
import matplotlib.pyplot as plt
import random
from heap import Heap

print(Heap)
G = nx.erdos_renyi_graph(10, 0.5)
for (u, v) in G.edges():
    G.edges[u, v]['weight'] = random.randint(1, 10)

pos = nx.spring_layout(G)
nx.draw(G, pos)
labels = nx.get_edge_attributes(G, 'weight')
nx.draw_networkx_edge_labels(
    G, pos, edge_labels=labels, alpha=0.5, edge_color='b')
# labels
nx.draw_networkx_labels(G, pos, font_size=10, font_family='sans-serif')
plt.show()
