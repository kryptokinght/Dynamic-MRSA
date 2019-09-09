from modules.graph import Graph


def main():
    """Driver program to test the above functions"""
    graph = Graph(10)
    # graph.shortestDist = graph.dijkstra(0)
    path = graph.getShortestPath(1, 8)
    graph.show_graph()
    # print(path)


if __name__ == '__main__':
    main()
