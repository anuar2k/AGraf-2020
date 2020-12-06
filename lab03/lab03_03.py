import os, sys, queue, functools, time
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

@functools.total_ordering
class ReverseItem:
    def __init__(self, key, val):
        self.key = key
        self.val = val

    def __eq__(self, other):
        return self.key == other.key

    def __lt__(self, other):
        return self.key > other.key

    def __repr__(self):
        return f"ReverseItem<{self.key}, {self.val}>"

def merge_vertices(G, src, tgt):
    for v, w in G[src].edges.items():
        G[tgt].add_edge(v, w)
        G[v].add_edge(tgt, w)

        G[v].del_edge(src)

    G[tgt].del_edge(tgt)
    G.pop(src)

def minimum_cut_phase(G):
    fst_elem_key = next(iter(G.keys()))
    C = {fst_elem_key:G[fst_elem_key]}

    Q = queue.PriorityQueue()
    cut_weights = {}
    for u in G:
        if u not in C:
            init_weight = sum([w for v, w in G[u].edges.items() if v in C])
            Q.put(ReverseItem(init_weight, u))
            cut_weights[u] = init_weight

    cut_weight = None
    while len(C) < len(G):
        u = None
        # skip queue elements, which were already used in C
        while True:
            item = Q.get()
            cut_weight, u = item.key, item.val
            if u not in C:
                break

        for v, w in G[u].edges.items():
            if v not in C:
                cut_weights[v] += w
                Q.put(ReverseItem(cut_weights[v], v))

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

data = dimacs.loadWeightedGraph("graphs/clique200")

t_start = time.process_time()
print(minimum_cut(*data))
print(time.process_time() - t_start)
