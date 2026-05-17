#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <atomic>
#include <csignal>
#include <set>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;
using namespace std::chrono;

// External termination signal
atomic<bool> signal_received(false);

// Signal handler function
void handle_signal(int) {
    signal_received = true;
}

class Graph {
public:
    int n;
    vector<vector<int>> adj;

    Graph(int nodes) : n(nodes), adj(nodes) {}

    void add_edge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    static Graph read_from_stdin() {
        string line;
        int n = 0, m = 0;
        while (getline(cin, line)) {
            if (line.empty() || line[0] == 'c')
                continue;
            if (line[0] == 'p') {
                stringstream ss(line);
                string tmp;
                ss >> tmp >> tmp >> n >> m;
                break;
            }
        }
        if (n == 0)
            return Graph(0);
        Graph g(n);
        int u, v;
        while (cin >> u >> v) {
            g.add_edge(u - 1, v - 1);
        }
        return g;
    }
};

// Solver implementing a basic Greedy algorithm.
// Its strategy is that at each step he picks the node that covers the most uncovered neighbors.
class Solver {
    const Graph& g;
    mt19937 rng;

public:
    Solver(const Graph& graph) : g(graph) {
        rng.seed(steady_clock::now().time_since_epoch().count());
    }

    // Detects and removes redundant nodes from the solution
    vector<int> clean(vector<int>& solution) {
        vector<int> count(g.n, 0);
        for (int u : solution) {
            count[u]++;
            for (int v : g.adj[u]) count[v]++;
        }

        shuffle(solution.begin(), solution.end(), rng);

        vector<int> optimized_sol;
        for (int u : solution) {
            bool needed = false;
            if (count[u] == 1)
                needed = true;
            else {
                for (int v : g.adj[u]) {
                    if (count[v] == 1) {
                        needed = true;
                        break;
                    }
                }
            }

            if (needed) {
                optimized_sol.push_back(u);
            } else {
                count[u]--;
                for (int v : g.adj[u]) count[v]--;
            }
        }
        return optimized_sol;
    }

    // Randomised greedy algo
    // - Assign a score to each node based on its degree
    // - Add random noise to the score to break ties and diversify selection
    // - Pick the node with the highest score, add to solution, mark neighbors
    // - Repeat until the graph is fully covered
    vector<int> solve() {
        vector<int> solution;
        vector<bool> covered(g.n, false);
        int covered_count = 0;
        set<pair<double, int>> pq;

        for (int i = 0; i < g.n; ++i) {
            double score = 1.0 + g.adj[i].size();
            double noise = (double)(rng() % 1000) / 100000.0;
            pq.insert({-(score + noise), i});
        }

        while (covered_count < g.n) {
            if (pq.empty())
                break;
            int u = pq.begin()->second;
            pq.erase(pq.begin());

            solution.push_back(u);

            if (!covered[u]) {
                covered[u] = true;
                covered_count++;
            }
            for (int v : g.adj[u]) {
                if (!covered[v]) {
                    covered[v] = true;
                    covered_count++;
                }
            }
        }

        if (covered_count < g.n) {
            for(int i=0; i < g.n; ++i) {
                if(!covered[i]) {
                    solution.push_back(i);
                }
            }
        }
        return clean(solution);
    }

    vector<int> improveSol(const vector<int>& base_solution) {
        vector<int> current_sol = base_solution;
        vector<bool> covered(g.n, false);

        if (current_sol.size() > 5) {
            shuffle(current_sol.begin(), current_sol.end(), rng);

            double percent = 0.10 + (rng() % 40) / 100.0;
            int remove_count = max(1, (int)(current_sol.size() * percent));

            for(int k=0; k < remove_count; ++k) {
                current_sol.pop_back();
            }

        } else {
            return solve();
        }

        vector<bool> in_sol(g.n, false);
        int covered_count = 0;

        for (int u : current_sol) {
            in_sol[u] = true;
            if (!covered[u]) {
                covered[u] = true;
                covered_count++;
            }
            for (int v : g.adj[u]) {
                if (!covered[v]) {
                    covered[v] = true;
                    covered_count++;
                }
            }
        }

        set<pair<double, int>> pq;

        for (int i = 0; i < g.n; ++i) {
            if (in_sol[i])
                continue;

            if (covered[i]) {
                int useful_degree = 0;
                for(int v : g.adj[i])
                    if(!covered[v])
                        useful_degree++;

                if (useful_degree > 0) {
                    double score = useful_degree;
                    double noise = (double)(rng() % 1000) / 100000.0;
                    pq.insert({-(score + noise), i});
                }
            } else {
                int useful_degree = 1;
                for(int v : g.adj[i])
                    if(!covered[v])
                        useful_degree++;

                double score = useful_degree;
                double noise = (double)(rng() % 1000) / 100000.0;
                pq.insert({-(score + noise), i});
            }
        }

        while (covered_count < g.n) {
            if (pq.empty()) {

                for(int i=0; i<g.n; ++i) {
                    if(!covered[i] && !in_sol[i]) {
                        current_sol.push_back(i);
                        in_sol[i] = true;
                        covered[i]=true; covered_count++;
                    }
                }
                break;
            }

            int u = pq.begin()->second;
            pq.erase(pq.begin());

            if (in_sol[u])
                continue;

            bool is_useful = false;
            if (!covered[u]) is_useful = true;
            else {
                for(int v : g.adj[u]) {
                    if(!covered[v]) {
                        is_useful = true;
                        break;
                    }
                }
            }

            if (is_useful) {
                current_sol.push_back(u);
                in_sol[u] = true;
                if (!covered[u]) {
                    covered[u] = true;
                    covered_count++;
                }
                for (int v : g.adj[u]) {
                    if (!covered[v]) {
                        covered[v] = true;
                        covered_count++;
                    }
                }
            }
        }
        return clean(current_sol);
    }
};

int main() {
    ios_base::sync_with_stdio(false); cin.tie(NULL);
    signal(SIGTERM, handle_signal);

    auto start_time = high_resolution_clock::now();
    const int TIME_LIMIT_SECONDS = 300;

    Graph g = Graph::read_from_stdin();
    if (g.n == 0) {
        cout << "0\n"; return 0;
    }

    Solver solver(g);

    vector<int> best_solution = solver.solve();

    while (!signal_received) {
        auto current_time = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(current_time - start_time).count();
        if (duration >= TIME_LIMIT_SECONDS)
            break;

        vector<int> candidate;

        if (rand() % 50 == 0) {
            candidate = solver.solve();
        } else {
            candidate = solver.improveSol(best_solution);
        }

        if (candidate.size() < best_solution.size()) {
            best_solution = candidate;
        } else if (candidate.size() == best_solution.size()) {
            if (rand() % 100 < 10)
                best_solution = candidate;
        }
    }

    cout << best_solution.size() << "\n";
    for (int v : best_solution)
        cout << (v + 1) << "\n";

    return 0;
}