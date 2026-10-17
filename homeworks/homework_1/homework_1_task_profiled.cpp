#define PROFILING_MODE
#include <chrono>
#include <iostream>

#include "homework_1_task.cpp"

int main() {
    size_t N = 0;
    std::cin >> N;
    const size_t RUNS = 100;

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < RUNS; i++) {
        StateNode::idMask = 0;
        StateNode startStateNode = BUILD::buildNode(N, StateNodeType::START);
        StateNode endStateNode = BUILD::buildNode(N, StateNodeType::END);

        std::unordered_map<size_t, std::unordered_set<size_t>> graph;
        std::unordered_set<std::string> visited;
        std::vector<StateNode> nodePath;

        dfs(graph, visited, nodePath, startStateNode, endStateNode);
    }
    auto end = std::chrono::high_resolution_clock::now();

    double averageMs = std::chrono::duration<double, std::milli>(end - start).count() / RUNS;
    std::cout << "# TIMES_MS: alg=" << averageMs << std::endl;

    return 0;
}
