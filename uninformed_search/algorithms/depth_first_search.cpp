#include <iostream>
#include <unordered_map>
#include <unordered_set>

void dfs(std::unordered_map<int, std::unordered_set<int>>& graph,
    std::unordered_set<int>& visited, const int currentNode, const int goalNode) {
    if (currentNode == goalNode) {
        return;
    }
    if (visited.find(currentNode) != visited.end()) {
        return;
    }

    visited.insert(currentNode);
    for (const int currentChild: graph[currentNode]) {
        if (visited.find(currentChild) == visited.end()) {
            dfs(graph, visited, currentChild, goalNode);
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
    dfs(graph, visited, 0, 3);
    return 0;
}