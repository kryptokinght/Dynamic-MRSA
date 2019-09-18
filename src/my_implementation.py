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


def createLightTree(paths):
    root = Tree()
    root.data = 1
    for path in paths:
        addPathToTree(path[1:], root)


def main():
    """Driver program to test the above functions"""
    graph = Graph(14, "network_graph.txt")
    paths = graph.getShortestPath(1, [2, 5, 8, 12, 13])
    # print(paths)
    links = graph.getAllEdges()
    linkIndexMap = {str(links[i][0])+":"+str(links[i][1]): i for i in range(len(links))}
    slots = 320
    spectrum_matrix = [[0 for j in range(slots)] for i in range(len(links))]
    # print(spectrum_matrix)
    """ root = createLightTree(paths)
    printTree(root) """


if __name__ == '__main__':
    main()
