import os, sys
import dimacs

os.chdir(sys.path[0])

class Node:
    def __init__(self, idx):
        self.idx = idx
        self.out = set()

    def connect_to(self, v):
        self.out.add(v)

# renumbers 1-based graph to 0-based
def renumber(n, edges):
    return n, [(edge[0] - 1, edge[1] - 1, edge[2]) for edge in edges]

def loadGraph(path):
    n, edges = renumber(*dimacs.loadWeightedGraph(path))
    vertices = [Node(vtx) for vtx in range(n)]

    for u, v, _ in edges:
        vertices[u].connect_to(v)
        vertices[v].connect_to(u)

    return vertices

def lexbfsNaive(graph):
    sets = [set(range(len(graph)))]
    visited = []

    while len(sets) > 0:
        curr = sets[-1].pop()

        visited.append(curr)

        partition = []
        for to_part in sets:
            reachable = to_part & graph[curr].out
            unreachable = to_part - graph[curr].out

            if len(unreachable) > 0:
                partition.append(unreachable)
            if len(reachable) > 0:
                partition.append(reachable)

        sets = partition

    return visited

def maxclique(graph):
    order = lexbfsNaive(graph)
    result = 1
    for i in range(len(order) - 1, -1, -1):
        if i < result:
            break
        v = order[i]
        cnt = len([u for u in order[:i] if u in graph[v].out]) + 1
        if cnt > result:
            result = cnt
    return result

# ordered list of members of RN(v), last member is parent(v)
def RN(graph, order, idx):
    v = order[idx]
    rn = [u for u in order[:idx] if u in graph[v].out]
    return rn

graph = loadGraph("maxclique/interval-rnd30")
print(maxclique(graph))