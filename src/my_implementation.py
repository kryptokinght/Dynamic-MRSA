from modules.graph import Graph
import random


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

def getReq(V,maxSlots):
    """select source"""
    source=random.randrange(0, V)
    # print(source)
    """select no of destinations"""
    noOfDestinations=random.randrange(1,V)
    # print(noOfDestinations)
    """random destinations"""
    listofPossibleDest=list(range(V))
    listofPossibleDest.remove(source)  #takes care of case that source is not in the destination vector
    # print(listofPossibleDest)
    destinations=random.sample( listofPossibleDest, noOfDestinations)
    # print(destinations)
    """no of slots needed by the request"""
    slotsNeeded=random.randrange(1,10)
    """Final request"""
    request=(source,destinations,slotsNeeded)
    # print(request)
    return request

def getCommonLinks(paths):
    links=set()
    for path in paths:
        for index,u in enumerate(path):
            if index<len(path)-1:
                links.add( ( path[index], path[index+1] ) )

    return links

def static_model(graph,spectrum_matrix,V,maxSlots):
    
    for i in range(10):
        
        request=getReq(V,maxSlots)
        source=request[0]
        destinations=request[1]
        slotsNeeded=request[2]

        paths = graph.getShortestPath(source,destinations)
        # print(paths)
        linksInLightTree=getCommonLinks(paths)
        print(linksInLightTree)
        


        
        




def main():
    """Driver program to test the above functions"""
    V=14
    graph = Graph(V, "network_graph.txt")
    # paths = graph.getShortestPath(1, [2, 5, 8, 12, 13])
    
    # print(paths)
    
    links = graph.getAllEdges()
    # print(links)
    linkIndexMap = {str(links[i][0])+":"+str(links[i][1]): i for i in range(len(links))}
    # print(linkIndexMap)
    slots = 320
    spectrum_matrix = [[0 for j in range(slots)] for i in range(len(links))]
    # print(spectrum_matrix)

    static_model(graph,spectrum_matrix,V,slots)

    """ root = createLightTree(paths)
    printTree(root) """


if __name__ == '__main__':
    main()
