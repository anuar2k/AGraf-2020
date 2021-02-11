#include <iostream>
#include <vector>
#include <unordered_set>

class Subset {
    public:
    Subset(Subset* prev, Subset* next, int first_vtx, int last_vtx) {
        this->prev = prev;
        this->next = next;
        this->first_vtx = first_vtx;
        this->last_vtx = last_vtx;

        if (this->prev != nullptr) {
            this->prev->next = this;
        }
        if (this->next != nullptr) {
            this->next->prev = this;
        }
    }

    ~Subset() {
        if (prev != nullptr) {
            prev->next = this->next;
        }
        if (next != nullptr) {
            next->prev = this->prev;
        }
    }

    bool empty() {
        return this->first_vtx > this->last_vtx;
    }

    Subset* prev;
    Subset* next;
    int first_vtx;
    int last_vtx;
    int last_part_epoch = -1;
};

struct Vertex {
    const int vtx;
    Subset* subset;
    int vtx_map_idx;
};

std::vector<int> lexbfs_order(std::vector<std::unordered_set<int>>& graph) {
    int n = graph.size();

    std::vector<Vertex> vertices;
    std::vector<Vertex*> vertices_map;
    vertices.reserve(n);
    vertices_map.reserve(n);

    Subset* initial_subset = new Subset(nullptr, nullptr, 0, n - 1);

    for (int vtx = 0; vtx < n; vtx++) {
        vertices.emplace_back(Vertex {
            .vtx = vtx,
            .subset = initial_subset,
            .vtx_map_idx = vtx
        });
        vertices_map.push_back(&vertices[vtx]);
    }

    for (int i = 0; i < n - 1; i++) {
        int curr_vtx = vertices_map[i]->vtx;

        //remove vertex from subset and remove the subset, if it's empty
        vertices_map[i]->subset->first_vtx += 1;

        if (vertices_map[i]->subset->empty()) {
            delete vertices_map[i]->subset;
        }

        vertices_map[i]->subset = nullptr;

        //partition
        for (int neighbor : graph[curr_vtx]) {
            Subset* neighbor_subset = vertices[neighbor].subset;

            if (neighbor_subset != nullptr) {
                //push the neighbor to the beggining of its subset
                int old_subset_first_map_idx = neighbor_subset->first_vtx;
                int old_neighbor_map_idx = vertices[neighbor].vtx_map_idx;

                std::swap(vertices[neighbor].vtx_map_idx, vertices_map[neighbor_subset->first_vtx]->vtx_map_idx);
                std::swap(vertices_map[old_subset_first_map_idx], vertices_map[old_neighbor_map_idx]);

                neighbor_subset->first_vtx += 1;

                //was this subset already partitioned in this iteration?
                if (neighbor_subset->last_part_epoch == i) {
                    //if yes, then move the element to previous subset
                    neighbor_subset->prev->last_vtx += 1;

                    vertices[neighbor].subset = neighbor_subset->prev;

                    //if old subset is empty, remove
                    if (neighbor_subset->empty()) {
                        delete neighbor_subset;
                    }
                }
                else {
                    //else create a new subset
                    neighbor_subset->last_part_epoch = i;
                    Subset* insert_after = neighbor_subset->prev;
                    Subset* insert_before;

                    //if old subset is empty, remove
                    if (neighbor_subset->empty()) {
                        insert_before = neighbor_subset->next;
                        delete neighbor_subset;
                    }
                    else {
                        insert_before = neighbor_subset;
                    }

                    vertices[neighbor].subset = new Subset(insert_after, insert_before, vertices[neighbor].vtx_map_idx, vertices[neighbor].vtx_map_idx);
                }
            }
        }
    }

    std::vector<int> result;
    result.reserve(n);
    for (int i = 0; i < n; i++) {
        result.push_back(vertices_map[i]->vtx);
    }

    delete vertices_map[n - 1]->subset;

    return result;
}

int maxclique(std::vector<std::unordered_set<int>>& graph, std::vector<int>& peo) {
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
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int game_count;
    std::cin >> game_count;

    while (game_count--) {
        int n, m;
        std::cin >> n >> m;

        std::vector<std::unordered_set<int>> graph(n, std::unordered_set<int>());
        while (m--) {
            int a, b;
            std::cin >> a >> b;

            a -= 1;
            b -= 1;

            graph[a].insert(b);
            graph[b].insert(a);
        }

        std::vector<int> order = lexbfs_order(graph);
        std::cout << std::max(2, maxclique(graph, order) - 1) << std::endl;
    }
}