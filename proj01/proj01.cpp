#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define INFINITY INT32_MAX
#define NONE -1

struct VertexData {
    int distance = INFINITY;
    int predecessor = NONE;
};

struct EdgeData {
    int capacity = 0;
    int cost = 0;
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
    vector<vector<int>> graph(vertex_count, vector<int>());

    //for instant lookup/update time
    vector<VertexData> vertex_data(vertex_count);
    vector<vector<EdgeData>> edge_data(vertex_count, vector<EdgeData>(vertex_count));

    //source to matches
    for (int match_vtx = matches_from; match_vtx < matches_to; ++match_vtx) {
        graph[source].push_back(match_vtx);
        graph[match_vtx].push_back(source);
    }

    //matches to players
    int match_vtx = matches_from;
    for (Match& match : tournament.matches) {
        graph[match_vtx].push_back(match.winner);
        graph[match.winner].push_back(match_vtx);

        graph[match_vtx].push_back(match.loser);
        graph[match.loser].push_back(match_vtx);

        edge_data[match_vtx][match.loser].cost = match.bribe_cost;
        edge_data[match.loser][match_vtx].cost = -match.bribe_cost;

        ++match_vtx;
    }

    //players (without king) to bottleneck
    for (int player_vtx = 1; player_vtx < tournament.player_count; ++player_vtx) {
        graph[player_vtx].push_back(bottleneck);
        graph[bottleneck].push_back(player_vtx);
    }

    //king to target
    graph[0].push_back(target);
    graph[target].push_back(0);

    //bottleneck to target
    graph[bottleneck].push_back(target);
    graph[target].push_back(bottleneck);

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
            edge_data[source][match_vtx].capacity = 1;
            edge_data[match_vtx][source].capacity = 0;
        }

        //matches to players
        match_vtx = matches_from;
        for (Match& match : tournament.matches) {
            edge_data[match_vtx][match.winner].capacity = 1;
            edge_data[match.winner][match_vtx].capacity = 0;

            edge_data[match_vtx][match.loser].capacity = 1;
            edge_data[match.loser][match_vtx].capacity = 0;
            ++match_vtx;
        }

        //players (without king) to bottleneck
        for (int player_vtx = 1; player_vtx < tournament.player_count; ++player_vtx) {
            edge_data[player_vtx][bottleneck].capacity = win_with;
            edge_data[bottleneck][player_vtx].capacity = 0;
        }

        //king to target
        edge_data[0][target].capacity = win_with;
        edge_data[target][0].capacity = 0;

        //bottleneck to target
        edge_data[bottleneck][target].capacity = tournament.match_count - win_with;
        edge_data[target][bottleneck].capacity = 0;

        //---------find augmenting paths with BF-----------
        int cost = 0;
        int flow = 0;
        while (true) {
            fill(vertex_data.begin(), vertex_data.end(), VertexData());
            vertex_data[source].distance = 0;

            int lowest_capacity = INFINITY;
            //|V| - 1 times
            for (int i = 0; i < vertex_count - 1; ++i) {
                //for every edge
                for (int u = 0; u < vertex_count; ++u) {
                    for (int v : graph[u]) {
                        if (edge_data[u][v].capacity > 0 && vertex_data[u].distance != INFINITY) {
                            int new_distance = vertex_data[u].distance + edge_data[u][v].cost;

                            if (new_distance < vertex_data[v].distance) {
                                vertex_data[v].distance = new_distance;
                                vertex_data[v].predecessor = u;

                                if (edge_data[u][v].capacity < lowest_capacity) {
                                    lowest_capacity = edge_data[u][v].capacity;
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
                    edge_data[pred][curr].capacity -= lowest_capacity;
                    edge_data[curr][pred].capacity += lowest_capacity;

                    cost += edge_data[pred][curr].cost * lowest_capacity;

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

    vector<Tournament> tournaments;

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
        tournaments.push_back(tournament);
    }

    for (Tournament& tournament : tournaments) {
        cout << (solve(tournament) ? "TAK" : "NIE") << endl;
    }
}