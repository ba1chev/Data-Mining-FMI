#include <iostream>
#include <unordered_map>
#include <unordered_set>

bool dfs(std::unordered_map<int, std::unordered_set<int>>& graph,
    std::unordered_set<int>& visited, const int currentNode, const int goalNode) {
    if (currentNode == goalNode) {
        return true;
    }
    if (visited.find(currentNode) != visited.end()) {
        return false;
    }

    visited.insert(currentNode);
    for (const int currentChild: graph[currentNode]) {
        if (visited.find(currentChild) == visited.end()) {
            if (dfs(graph, visited, currentChild, goalNode)) {
                return true;
            }
        }
    }
    return false;
}