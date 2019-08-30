from modules.graph import Graph


def main():
    """Driver program to test the above functions"""
    graph = Graph(9, 'network_graph.txt')
    #graph.shortestDist = graph.dijkstra(0)
    path = graph.getShortestPath(8, 4)
    print(path)


if __name__ == '__main__':
    main()
