import os, sys, collections
import dimacs

os.chdir(sys.path[0])

def capacity(graph, flow, u, v):
    if v in graph[u]:
        return graph[u][v] - flow[u][v]
    if u in graph[v]:
        return flow[v][u]

def dfs(size, graph, flow, source, target):
    visited = [False] * size

    def dfsVisit(u):
        visited[u] = True
        for v, _ in flow[u].items():
            if not visited[v] and capacity(graph, flow, u, v) > 0:
                if v == target:
                    return [(u, v)]
                if len(path := dfsVisit(v)) > 0:
                    path.append((u, v))
                    return path

        return []

    return dfsVisit(source)

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

def ff(size, edgeList):
    edgeList = list(map(lambda edge: (edge[0] - 1, edge[1] - 1, edge[2]), edgeList))
    graph = [dict() for _ in range(size)]
    flow = [dict() for _ in range(size)]

    source = 0
    target = size - 1

    for u, v, w in edgeList:
        graph[u][v] = w
        flow[u][v] = 0
        flow[v][u] = 0

    while len(path := dfs(size, graph, flow, source, target)) > 0:
        min_cap = capacity(graph, flow, *min(path, key=lambda edge: capacity(graph, flow, *edge)))

        for u, v in path:
            flow[u][v] = flow[u][v] + min_cap
            flow[v][u] = flow[v][u] - min_cap

    flow_value = 0
    for v in graph[source].keys():
        flow_value += flow[source][v]

    return flow_value

def ek(size, edgeList):
    edgeList = list(map(lambda edge: (edge[0] - 1, edge[1] - 1, edge[2]), edgeList))
    graph = [dict() for _ in range(size)]
    flow = [dict() for _ in range(size)]

    source = 0
    target = size - 1

    for u, v, w in edgeList:
        graph[u][v] = w
        flow[u][v] = 0
        flow[v][u] = 0

    while len(path := bfs(size, graph, flow, source, target)) > 0:
        min_cap = capacity(graph, flow, *min(path, key=lambda edge: capacity(graph, flow, *edge)))

        for u, v in path:
            flow[u][v] = flow[u][v] + min_cap
            flow[v][u] = flow[v][u] - min_cap

    flow_value = 0
    for v in graph[source].keys():
        flow_value += flow[source][v]

    return flow_value

data = dimacs.loadDirectedWeightedGraph("flow/clique100")
print(ff(*data))
print(ek(*data))