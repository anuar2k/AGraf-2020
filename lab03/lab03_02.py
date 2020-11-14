import os, sys
import dimacs

class Node:
    def __init__(self):
        self.edges = dict()

    def add_edge(self, target, weight):
        self.edges[target] = self.edges.get(target, 0) + weight

    def del_edge(self, target):
        del self.edges[target]

def merge_vertices(G, x, y):
    pass

def minimum_cut_phase(size, edge_list):
    edge_list = list(map(lambda edge: (edge[0] - 1, edge[1] - 1, edge[2]), edge_list))
    pass

data = dimacs.loadWeightedGraph("graphs/trivial")
print(minimum_cut_phase(*data))