from modules.graph import Graph


def main():
    """Driver program to test the above functions"""
    graph = Graph(30)
    path = graph.getShortestPath(1, 27)
    print(path)
    graph.show_graph()


if __name__ == '__main__':
    main()
