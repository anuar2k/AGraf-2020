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

def checkLexBFS(G, vs):
    n = len(G)
    pi = [None] * n
    for i, v in enumerate(vs):
        pi[v] = i

    for i in range(n-1):
        for j in range(i+1, n-1):
            Ni = G[vs[i]].out
            Nj = G[vs[j]].out

            verts = [pi[v] for v in Nj - Ni if pi[v] < i]
            if verts:
                viable = [pi[v] for v in Ni - Nj]
                if not viable or min(verts) <= min(viable):
                    return False
    return True

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

# ordered list of members of RN(v), last member is parent(v)
def RN(graph, order, idx):
    v = order[idx]
    rn = [u for u in order[:idx] if u in graph[v].out]
    return rn

def isChordal(graph):
    order = lexbfsNaive(graph)
    rnn = [RN(graph, order, i) for i in range(len(order))]

    rn = [None] * len(graph)
    for i in range(len(graph)):
        v = order[i]
        rn[v] = rnn[i]

    for vtx in range(len(order)):
        if len(rn[vtx]) > 0:
            parent = rn[vtx][-1]
            rn_without_parent = rn[vtx][:-1]

            if not set(rn_without_parent) <= set(rn[parent]):
                return False

    return True

graph = loadGraph("chordal/example-fig5")
order = lexbfsNaive(graph)
print(order)
print(checkLexBFS(graph, order))
print(isChordal(graph))

def perfect_order_elimination(graph, ordering):
    n = len(graph)
    RNs = [None for _ in range(n)]
    for idx, id_ in enumerate(ordering):
        ordering[idx] = graph[id_]

        befores = ordering[:idx]
        RNs[id_] = set(befores).intersection(graph[id_].neighbours)

        if not RNs[id_]:
            continue

        for vert in befores[::-1]:
            if vert in RNs[id_]:
                parent = vert
                break

        parentRN = RNs[parent.id]
        parentRN.add(parent)
        if not RNs[id_].issubset(parentRN):
            return False
    return True