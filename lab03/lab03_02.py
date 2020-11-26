import os, sys
import dimacs

os.chdir(sys.path[0])

class Node:
    def __init__(self):
        self.edges = {}

    def add_edge(self, tgt, w):
        self.edges[tgt] = self.edges.get(tgt, 0) + w

    def del_edge(self, tgt):
        self.edges.pop(tgt, None)

    def __repr__(self):
        return self.edges.__repr__()

def merge_vertices(G, src, tgt):
    for v, w in G[src].edges.items():
        G[tgt].add_edge(v, w)
        G[v].add_edge(tgt, w)

        G[v].del_edge(src)

    G[tgt].del_edge(tgt)
    G.pop(src)

def tightness(G, S):
    result = {}
    for u, node in G.items():
        if u in S:
            result[u] = 0
        else:
            result[u] = sum([w for v, w in node.edges.items() if v in S])

    return result

def minimum_cut_phase(G):
    fst_elem_key = next(iter(G.keys()))
    C = {fst_elem_key:G[fst_elem_key]}

    cut_weight = None
    while len(C) < len(G):
        u, cut_weight = max(tightness(G, C).items(), key = lambda item: item[1])
        C[u] = G[u]

    r_iter = reversed(G.keys())
    t = next(r_iter)
    s = next(r_iter)

    merge_vertices(G, s, t)

    return cut_weight

def minimum_cut(size, edge_list):
    edge_list = list(map(lambda edge: (edge[0] - 1, edge[1] - 1, edge[2]), edge_list))

    G = {i:Node() for i in range(size)}

    for u, v, w in edge_list:
        G[u].add_edge(v, w)
        G[v].add_edge(u, w)

    return min([minimum_cut_phase(G) for _ in range(size - 1)])

data = dimacs.loadWeightedGraph("graphs/grid100x100")
print(minimum_cut(*data))