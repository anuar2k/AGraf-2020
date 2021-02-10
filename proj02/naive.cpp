#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

using namespace std;

unordered_set<int> intersection(unordered_set<int>& a, unordered_set<int>& b) {
    unordered_set<int> result;

    for (int el : a) {
        if (b.find(el) != b.end()) {
            result.insert(el);
        }
    }

    return result;
}

unordered_set<int> difference(unordered_set<int>& a, unordered_set<int>& b) {
    unordered_set<int> result;

    for (int el : a) {
        if (b.find(el) == b.end()) {
            result.insert(el);
        }
    }

    return result;
}

vector<int> lexbfs_order(vector<unordered_set<int>>& graph) {
    int n = graph.size();
    vector<unordered_set<int>> queue(1, unordered_set<int>(n));
    vector<int> result_order;
    result_order.reserve(n);

    for (int vtx = 0; vtx < n; vtx++) {
        queue[0].insert(vtx);
    }

    while (queue.size() > 0) {
        int curr = *queue.back().begin();
        queue.back().erase(curr);

        result_order.push_back(curr);

        vector<unordered_set<int>> partition;
        for (unordered_set<int>& to_part : queue) {
            unordered_set<int> reachable = intersection(to_part, graph[curr]);
            unordered_set<int> unreachable = difference(to_part, graph[curr]);

            if (unreachable.size() > 0) {
                partition.emplace_back(unreachable);
            }

            if (reachable.size() > 0) {
                partition.emplace_back(reachable);
            }
        }

        queue = partition;
    }
    return result_order;
}

int maxclique(vector<unordered_set<int>>& graph, vector<int>& peo) {
    int n = peo.size();
    int result = 1;

    for (int i = n - 1; i >= 0; i--) {
        if (i < result) {
            break;
        }

        int curr = peo[i];

        int count = 1;
        for (int j = 0; j < i; j++) {
            if (graph[curr].find(peo[j]) != graph[curr].end()) {
                count++;
            }
        }

        if (count > result) {
            result = count;
        }
    }

    return result;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int game_count;
    cin >> game_count;

    while (game_count--) {
        int n, m;
        cin >> n >> m;

        vector<unordered_set<int>> graph(n, unordered_set<int>());
        while (m--) {
            int a, b;
            cin >> a >> b;

            a -= 1;
            b -= 1;

            graph[a].insert(b);
            graph[b].insert(a);
        }

        vector<int> order = lexbfs_order(graph);
        cout << max(2, maxclique(graph, order) - 1) << endl;
    }
}