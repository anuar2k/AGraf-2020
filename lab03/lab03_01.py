import os, sys, collections, math
import dimacs

os.chdir(sys.path[0])

def capacity(graph, flow, u, v):
    if v in graph[u]:
        return graph[u][v] - flow[u][v]
    if u in graph[v]:
        return flow[v][u]

def bfs(size, graph, flow, source, target):
    parent = [None] * size
    visited = [False] * size
    q = collections.deque()

    visited[source] = True
    q.append(source)

    while len(q) > 0:
        u = q.popleft()
        for v in flow[u].keys():
            if not visited[v] and capacity(graph, flow, u, v) > 0:
                parent[v] = u
                visited[v] = True
                
                q.append(v)

    path = []
    u = parent[target]
    v = target
    while u is not None:
        path.append((u, v))
        u = parent[u]
        v = parent[v]

    return path

def edge_cut(size, edge_list):
    edge_list = list(map(lambda edge: (edge[0] - 1, edge[1] - 1, edge[2]), edge_list))

    extended_size = size + len(edge_list)

    graph = [dict() for _ in range(extended_size)]
    flow = [dict() for _ in range(extended_size)]

    for (idx, (u, v, _)) in enumerate(edge_list):
        mid_vtx = size + idx

        # u to v
        graph[u][v] = 1
        flow[u][v] = 0
        flow[v][u] = 0

        # v to u with mid_vtx to not break the max-flow algorithm
        graph[v][mid_vtx] = 1
        flow[v][mid_vtx] = 0
        flow[mid_vtx][v] = 0

        graph[mid_vtx][u] = 1
        flow[mid_vtx][u] = 0
        flow[u][mid_vtx] = 0

    lowest = math.inf
    for target in range(1, size):
        for flow_ends in flow:
            for end in flow_ends.keys():
                flow_ends[end] = 0

        while len(path := bfs(extended_size, graph, flow, 0, target)) > 0:
            min_cap = capacity(graph, flow, *min(path, key=lambda edge: capacity(graph, flow, *edge)))

            for u, v in path:
                flow[u][v] = flow[u][v] + min_cap
                flow[v][u] = flow[v][u] - min_cap

        flow_value = sum([flow[0][v] for v in graph[0].keys()])
        lowest = min(lowest, flow_value)

    return lowest

data = dimacs.loadWeightedGraph("graphs/clique100")
print(edge_cut(*data))