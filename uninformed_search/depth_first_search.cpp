#include <iostream>
#include <unordered_map>
#include <unordered_set>

void dfs(std::unordered_map<int, std::unordered_set<int>>& graph,
    std::unordered_set<int>& visited, const int current) {
    if (visited.find(current) != visited.end()) {
        return;
    }

    std::cout << current << std::endl;
    visited.insert(current);
    for (const int currentChild: graph[current]) {
        if (visited.find(currentChild) == visited.end()) {
            dfs(graph, visited, currentChild);
        }
    }
}

int main() {
    std::unordered_map<int, std::unordered_set<int>> graph;
    for (size_t i = 0; i < 4; i++) {
        graph[i];
    }
    
    graph[0].insert(1);
    graph[0].insert(2);
    graph[0].insert(3);

    std::unordered_set<int> visited;
    dfs(graph, visited, 0);
    return 0;
}