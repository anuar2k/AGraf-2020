#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define INFINITY INT32_MAX
#define NONE -1

struct Vertex {
    int distance = INFINITY;
    int predecessor = NONE;
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

bool solve(Tournament& tournament) {
    //-------------initialize graph and edge costs---------

    //vertex ranges
    int players_from = 0;
    int players_to = players_from + tournament.player_count; //exclusive
    int matches_from = players_to;
    int matches_to = matches_from + tournament.match_count; //exclusive
    int bottleneck = matches_to;
    int source = bottleneck + 1;
    int target = source + 1;
    int vertex_count = target + 1;

    //for optimised iteration time
    vector<vector<Edge>> graph(vertex_count, vector<Edge>());

    //for instant lookup/update time
    vector<Vertex> vertex_data(vertex_count);

    //source to matches
    for (int match_vtx = matches_from; match_vtx < matches_to; ++match_vtx) {
        graph[source].emplace_back(match_vtx);
        graph[match_vtx].emplace_back(source);
    }

    //matches to players
    int match_vtx = matches_from;
    for (Match& match : tournament.matches) {
        graph[match_vtx].emplace_back(match.winner);
        graph[match.winner].emplace_back(match_vtx);

        graph[match_vtx].emplace_back(match.loser, match.bribe_cost);
        graph[match.loser].emplace_back(match_vtx, -match.bribe_cost);

        ++match_vtx;
    }

    //players (without king) to bottleneck
    for (int player_vtx = 1; player_vtx < tournament.player_count; ++player_vtx) {
        graph[player_vtx].emplace_back(bottleneck);
        graph[bottleneck].emplace_back(player_vtx);
    }

    //king to target
    graph[0].emplace_back(target);
    graph[target].emplace_back(0);

    //bottleneck to target
    graph[bottleneck].emplace_back(target);
    graph[target].emplace_back(bottleneck);

    //-------------actual algorithm------------------------
    int won_by_king = 0;
    for (Match& match : tournament.matches) {
        if (match.winner == 0) {
            ++won_by_king;
        }
    }

    for (int win_with = max(won_by_king, tournament.player_count / 2); win_with < tournament.player_count; ++win_with) {
        //-----------set capacities in graph---------------

        //source to matches
        for (int match_vtx = matches_from; match_vtx < matches_to; ++match_vtx) {
            find_edge(graph[source], match_vtx).capacity = 1;
            find_edge(graph[match_vtx], source).capacity = 0;
        }

        //matches to players
        match_vtx = matches_from;
        for (Match& match : tournament.matches) {
            find_edge(graph[match_vtx], match.winner).capacity = 1;
            find_edge(graph[match.winner], match_vtx).capacity = 0;

            find_edge(graph[match_vtx], match.loser).capacity = 1;
            find_edge(graph[match.loser], match_vtx).capacity = 0;
            ++match_vtx;
        }

        //players (without king) to bottleneck
        for (int player_vtx = 1; player_vtx < tournament.player_count; ++player_vtx) {
            find_edge(graph[player_vtx], bottleneck).capacity = win_with;
            find_edge(graph[bottleneck], player_vtx).capacity = 0;
        }

        //king to target
        find_edge(graph[0], target).capacity = win_with;
        find_edge(graph[target], 0).capacity = 0;

        //bottleneck to target
        find_edge(graph[bottleneck], target).capacity = tournament.match_count - win_with;
        find_edge(graph[target], bottleneck).capacity = 0;

        //---------find augmenting paths with BF-----------
        int cost = 0;
        int flow = 0;
        while (true) {
            fill(vertex_data.begin(), vertex_data.end(), Vertex());
            vertex_data[source].distance = 0;

            int lowest_capacity = INFINITY;
            //|V| - 1 times
            for (int i = 0; i < vertex_count - 1; ++i) {
                //for every edge
                for (int u = 0; u < vertex_count; ++u) {
                    for (Edge& edge : graph[u]) {
                        if (edge.capacity > 0 && vertex_data[u].distance != INFINITY) {
                            int new_distance = vertex_data[u].distance + edge.cost;

                            if (new_distance < vertex_data[edge.target].distance) {
                                vertex_data[edge.target].distance = new_distance;
                                vertex_data[edge.target].predecessor = u;

                                if (edge.capacity < lowest_capacity) {
                                    lowest_capacity = edge.capacity;
                                }
                            }
                        }
                    }
                }
            }

            int curr = target;
            int pred = vertex_data[target].predecessor;

            if (pred != NONE) {
                flow += lowest_capacity;
                
                while (pred != NONE) {
                    find_edge(graph[pred], curr).capacity -= lowest_capacity;
                    find_edge(graph[curr], pred).capacity += lowest_capacity;

                    cost += find_edge(graph[pred], curr).cost * lowest_capacity;

                    curr = vertex_data[curr].predecessor;
                    pred = vertex_data[pred].predecessor;
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

        for (int j = 0; j < tournament.match_count; ++j) {
            Match match;
            int p1, p2, winner, bribe_cost;

            cin >> p1 >> p2 >> winner >> bribe_cost;

            match.winner = p1 == winner ? p1 : p2;
            match.loser = p1 == winner ? p2 : p1;
            match.bribe_cost = bribe_cost;

            tournament.matches.push_back(match);
        }

        cout << (solve(tournament) ? "TAK" : "NIE") << endl;
    }
}