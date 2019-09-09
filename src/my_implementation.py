from modules.graph import Graph


class Tree:
    def __init__(self):
        self.children = []
        self.data = None


def addPathToTree(path, root):
    """ Creates multicast tree"""
    if not path:
        return
    found = 0
    for child in root.children:
        if child.data == path[0]:
            found = 1
            addPathToTree(path[1:], child)
    if not found:
        temp = Tree()
        temp.data = path[0]
        root.children.append(temp)
        addPathToTree(path[1:], temp)


def printTree(root):
    """performs DFS and prints the tree"""
    if root is None:
        return
    children = [child.data for child in root.children]
    print("root: {} children: {}".format(root.data, children))
    for child in root.children:
        printTree(child)


def main():
    """Driver program to test the above functions"""
    graph = Graph(20)
    paths = graph.getShortestPath(1, [3, 6, 8, 15, 19])
    print(paths)

    root = Tree()
    root.data = 1
    for path in paths:
        addPathToTree(path[1:], root)

    printTree(root)
    graph.show_graph()


if __name__ == '__main__':
    main()
