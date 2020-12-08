#include <iostream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

#define INFINITY INT32_MAX
#define NONE -1

struct Vertex {
    int distance = INFINITY;
    int predecessor = NONE;
    int start_potential;
    int potential;
    bool reached;
};

struct VertexRanges {
    int players_from;
    int players_to;
    int bottleneck;
    int source;
    int target;
    int vertex_count;
};

struct Edge {
    int target;
    int cost;
    int capacity;

    Edge(int target) : target(target), cost(0), capacity(0) { }
    Edge(int target, int cost) : target(target), cost(cost), capacity(0) { }
};

struct Match {
    int winner;
    int loser;
    int bribe_cost;
};

struct Tournament {
    int budget;
    int player_count;
    int match_count;
    vector<Match> matches;
    vector<int> win_count;
};

int n_choose_2(int n) {
    return n * (n - 1) / 2;
}

int max(int a, int b) {
    return a > b ? a : b;
}

//you shouldn't try to find an unexisting edge
inline Edge& find_edge(vector<Edge>& edge_list, int target) {
    for (Edge& edge : edge_list) {
        if (edge.target == target) {
            return edge;
        }
    }

    //mmm, dereferencing nullpointers! moje ulubiene, pyszne byli!
    return *(Edge*)nullptr;
}

void init_edges(Tournament& tournament, VertexRanges& vr, vector<vector<Edge>>& graph) {
    //source to players
    for (int player_vtx = 0; player_vtx < tournament.player_count; ++player_vtx) {
        graph[vr.source].emplace_back(player_vtx);
        graph[player_vtx].emplace_back(vr.source);
    }

    //matches (player gives up for bribe_cost money)
    for (Match& match : tournament.matches) {
        if (match.winner != 0 && match.bribe_cost <= tournament.budget) {
            graph[match.winner].emplace_back(match.loser, match.bribe_cost);
            graph[match.loser].emplace_back(match.winner, -match.bribe_cost);
        }
    }

    //players (without king) to bottleneck
    for (int player_vtx = 1; player_vtx < tournament.player_count; ++player_vtx) {
        graph[player_vtx].emplace_back(vr.bottleneck);
        graph[vr.bottleneck].emplace_back(player_vtx);
    }

    //king to target
    graph[0].emplace_back(vr.target);
    graph[vr.target].emplace_back(0);

    //bottleneck to target
    graph[vr.bottleneck].emplace_back(vr.target);
    graph[vr.target].emplace_back(vr.bottleneck);
}

void reset_capacities(Tournament& tournament, VertexRanges& vr, vector<vector<Edge>>& graph, int win_with) {
    //source to players
    for (int player_vtx = 0; player_vtx < tournament.player_count; ++player_vtx) {
        find_edge(graph[vr.source], player_vtx).capacity = tournament.win_count[player_vtx];
        find_edge(graph[player_vtx], vr.source).capacity = 0;
    }

    //matches (player gives up for bribe_cost money)
    for (Match& match : tournament.matches) {
        if (match.winner != 0 && match.bribe_cost <= tournament.budget) {
            find_edge(graph[match.winner], match.loser).capacity = 1;
            find_edge(graph[match.loser], match.winner).capacity = 0;
        }
    }

    //players (without king) to bottleneck
    for (int player_vtx = 1; player_vtx < tournament.player_count; ++player_vtx) {
        find_edge(graph[player_vtx], vr.bottleneck).capacity = win_with;
        find_edge(graph[vr.bottleneck], player_vtx).capacity = 0;
    }

    //king to target
    find_edge(graph[0], vr.target).capacity = win_with;
    find_edge(graph[vr.target], 0).capacity = 0;

    //bottleneck to target
    find_edge(graph[vr.bottleneck], vr.target).capacity = tournament.match_count - win_with;
    find_edge(graph[vr.target], vr.bottleneck).capacity = 0;
}

bool solve(Tournament& tournament) {
    //-------------initialize graph and edge costs---------
    VertexRanges vr {
        .players_from = 0,
        .players_to = vr.players_from + tournament.player_count,
        .bottleneck = vr.players_to + 1,
        .source = vr.bottleneck + 1,
        .target = vr.source + 1,
        .vertex_count = vr.target + 1
    };

    vector<vector<Edge>> graph(vr.vertex_count);
    vector<Vertex> vertex_data(vr.vertex_count);

    init_edges(tournament, vr, graph);

    //-------------actual algorithm------------------------

    //find potentials using Bellman-Ford
    reset_capacities(tournament, vr, graph, 1);

    fill(vertex_data.begin(), vertex_data.end(), Vertex());
    vertex_data[vr.source].distance = 0;

    //|V| - 1 times
    for (int i = 0; i < vr.vertex_count - 1; ++i) {
        //for every edge
        for (int u = 0; u < vr.vertex_count; ++u) {
            for (Edge& edge : graph[u]) {
                if (edge.capacity > 0 && vertex_data[u].distance != INFINITY) {
                    int new_distance = vertex_data[u].distance + edge.cost;

                    if (new_distance < vertex_data[edge.target].distance) {
                        vertex_data[edge.target].distance = new_distance;
                    }
                }
            }
        }
    }

    for (int vtx = 0; vtx < vr.vertex_count; ++vtx) {
        vertex_data[vtx].start_potential = vertex_data[vtx].distance;
    }

    //get lower bound for win_with
    int won_by_king = 0;
    for (Match& match : tournament.matches) {
        if (match.winner == 0) {
            ++won_by_king;
        }
    }

    for (int win_with = max(won_by_king, tournament.player_count / 2); win_with < tournament.player_count; ++win_with) {
        //-----------set capacities in graph---------------
        reset_capacities(tournament, vr, graph, win_with);

        //---find augmenting paths with modified Dijkstra--
        int cost = 0;
        int flow = 0;

        //set initial potentials
        for (int vtx = 0; vtx < vr.vertex_count; ++vtx) {
            vertex_data[vtx].potential = vertex_data[vtx].start_potential;
        }

        while (true) {
            for (int vtx = 0; vtx < vr.vertex_count; ++vtx) {
                vertex_data[vtx].distance = INFINITY;
                vertex_data[vtx].predecessor = NONE;
                vertex_data[vtx].reached = false;
            }

            vertex_data[vr.source].distance = 0;

            int lowest_capacity = INFINITY;
            //lexicographical comparison of pairs, so weight goes first
            priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> queue;
            queue.emplace(0, vr.source);

            while (!queue.empty()) {
                pair<int, int> dist_vtx = queue.top();
                queue.pop();

                int u = dist_vtx.second;

                if (vertex_data[u].reached || dist_vtx.first != vertex_data[u].distance) {
                    continue;
                }

                vertex_data[u].reached = true;

                for (Edge& edge : graph[u]) {
                    if (edge.capacity > 0) {
                        int new_distance = vertex_data[u].distance + edge.cost + vertex_data[u].potential - vertex_data[edge.target].potential;

                        if (new_distance < vertex_data[edge.target].distance) {
                            vertex_data[edge.target].distance = new_distance;
                            vertex_data[edge.target].predecessor = u;

                            if (edge.capacity < lowest_capacity) {
                                lowest_capacity = edge.capacity;
                            }

                            queue.emplace(new_distance, edge.target);
                        }
                    }
                }

            }

            int curr = vr.target;
            int pred = vertex_data[vr.target].predecessor;

            if (pred != NONE) {
                //update flow
                flow += lowest_capacity;
                
                while (pred != NONE) {
                    find_edge(graph[pred], curr).capacity -= lowest_capacity;
                    find_edge(graph[curr], pred).capacity += lowest_capacity;

                    cost += find_edge(graph[pred], curr).cost * lowest_capacity;

                    curr = vertex_data[curr].predecessor;
                    pred = vertex_data[pred].predecessor;
                }
                
                //update potentials
                for (int vtx = 0; vtx < vr.vertex_count; ++vtx) {
                    if (vertex_data[vtx].distance == INFINITY) {
                        vertex_data[vtx].potential = INFINITY;
                    }
                    else {
                        vertex_data[vtx].potential += vertex_data[vtx].distance;
                    }
                }
            }
            else {
                break;
            }
        }

        if (flow == tournament.match_count && cost <= tournament.budget) {
            return true;
        }
    }

    return false;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int tournament_count;
    cin >> tournament_count;

    for (int i = 0; i < tournament_count; ++i) {
        Tournament tournament;

        cin >> tournament.budget;
        cin >> tournament.player_count;

        tournament.match_count = n_choose_2(tournament.player_count);
        tournament.win_count.resize(tournament.player_count);

        for (int j = 0; j < tournament.match_count; ++j) {
            Match match;
            int p1, p2, winner, bribe_cost;

            cin >> p1 >> p2 >> winner >> bribe_cost;

            match.winner = p1 == winner ? p1 : p2;
            match.loser = p1 == winner ? p2 : p1;
            match.bribe_cost = bribe_cost;

            tournament.matches.push_back(match);
            tournament.win_count[match.winner] += 1;
        }

        cout << (solve(tournament) ? "TAK" : "NIE") << endl;
    }
}
