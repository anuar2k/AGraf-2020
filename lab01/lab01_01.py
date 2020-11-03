import os, sys
import dimacs

os.chdir(sys.path[0])

class Vertex:
    def __init__(self):
        self.parent = self
        self.rank = 0

class VertexSets:
    def __init__(self, size):
        self.size = size
        self.vertices = [Vertex() for _ in range(size)]

    def __find_set(self, x):
        if x is not x.parent:
            x.parent = self.__find_set(x.parent)
        return x.parent

    def __merge(self, x, y):
        x = self.__find_set(x)
        y = self.__find_set(y)
        if x.rank > y.rank:
            y.parent = x
        else:
            x.parent = y
            if x.rank == y.rank:
                y.rank += 1

    def union(self, x, y):
        self.__merge(self.vertices[x], self.vertices[y])
        
    def same_set(self, x, y):
        return self.__find_set(self.vertices[x]) is self.__find_set(self.vertices[y])


def findPathUnion(V, L, s, t):
    s = s - 1
    t = t - 1
    L = sorted(list(map(lambda edge: (edge[0] - 1, edge[1] - 1, edge[2]), L)), key=lambda edge: edge[2])

    result = None
    vs = VertexSets(V)
    while not vs.same_set(s, t):
        toMerge = L.pop()
        result = toMerge[2]
        vs.union(toMerge[0], toMerge[1])

    return result

(V, L) = dimacs.loadWeightedGraph("graphs/g1")
print(findPathUnion(V, L, 1, 2))

(V, L) = dimacs.loadWeightedGraph("graphs/rand20_100")
print(findPathUnion(V, L, 1, 2))

(V, L) = dimacs.loadWeightedGraph("graphs/pp1000")
print(findPathUnion(V, L, 1, 2))

(V, L) = dimacs.loadWeightedGraph("graphs/rand1000_100000")
print(findPathUnion(V, L, 1, 2))


