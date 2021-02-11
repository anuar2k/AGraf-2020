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
    int last_part_epoch = -1; //tracks if this subset was partitioned in current partition process
};

struct Vertex {
    const int vtx;
    Subset* subset;
    int vtx_map_idx; //index of the vertex in vertices_map
};

int chordal_maxclique(std::vector<std::unordered_set<int>>& graph) {
    int n = graph.size();

    std::vector<Vertex> vertices; //allocates Vertices, allows O(1) access by absolute index
    std::vector<Vertex*> vertices_map; //allows O(1) access in reordered subsets

    //reserve n, to make sure that pointers to vertices' elements will not be invalidated during push_backs
    vertices.reserve(n);
    vertices_map.reserve(n);

    //add all vertices to the initial subset
    Subset* initial_subset = new Subset(nullptr, nullptr, 0, n - 1);
    for (int vtx = 0; vtx < n; vtx++) {
        vertices.emplace_back(Vertex {
            .vtx = vtx,
            .subset = initial_subset,
            .vtx_map_idx = vtx
        });
        vertices_map.push_back(&vertices[vtx]);
    }

    int maxclique = 1;
    for (int i = 0; i < n - 1; i++) {
        //select the vertex from the leftmost subset
        int curr_vtx = vertices_map[i]->vtx;

        //remove vertex from subset and remove the subset, if it's empty
        vertices_map[i]->subset->first_vtx += 1;
        if (vertices_map[i]->subset->empty()) {
            delete vertices_map[i]->subset;
        }

        vertices_map[i]->subset = nullptr;

        int curr_clique = 1;
        //subset partition
        for (int neighbor : graph[curr_vtx]) {
            Subset* neighbor_subset = vertices[neighbor].subset;

            if (neighbor_subset == nullptr) {
                curr_clique += 1;
            }
            else {
                //push the neighbor to the beginning of its subset by swapping
                int old_subset_first_idx = neighbor_subset->first_vtx;
                int old_neighbor_idx = vertices[neighbor].vtx_map_idx;

                std::swap(vertices[neighbor].vtx_map_idx, vertices_map[neighbor_subset->first_vtx]->vtx_map_idx);
                std::swap(vertices_map[old_subset_first_idx], vertices_map[old_neighbor_idx]);

                neighbor_subset->first_vtx += 1;

                //was this subset already partitioned in this iteration?
                if (neighbor_subset->last_part_epoch == i) { //if yes, then move the element to previous subset
                    neighbor_subset->prev->last_vtx += 1;

                    vertices[neighbor].subset = neighbor_subset->prev;

                    //if old subset is empty, remove it
                    if (neighbor_subset->empty()) {
                        delete neighbor_subset;
                    }
                }
                else { //else create a new subset
                    //note, that old subset was already partitioned in this iteration
                    neighbor_subset->last_part_epoch = i;

                    //insert a new subset, and if the old one is empty, remove it
                    Subset* insert_after = neighbor_subset->prev;
                    Subset* insert_before;

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

        if (curr_clique > maxclique) {
            maxclique = curr_clique;
        }
    }

    delete vertices_map[n - 1]->subset;

    return maxclique;
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

        std::cout << std::max(2, chordal_maxclique(graph) - 1) << std::endl;
    }
}