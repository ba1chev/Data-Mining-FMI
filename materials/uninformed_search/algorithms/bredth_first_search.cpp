#include <queue>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

bool bfs(std::unordered_map<int, std::unordered_set<int>>& graph, 
    const int startNode, const int goalNode) {
    if (startNode == goalNode) {
        return true;
    }

    std::queue<int> q;
    std::unordered_set<int> visited;
    
    q.push(startNode);
    visited.insert(startNode);
    while (!q.empty()) {
        const int current = q.front();
        q.pop();
        if (current == goalNode) {
            return true;
        }

        for (const int currentChildNode: graph[current]) {
            if (visited.find(currentChildNode) == visited.end()) {
                q.push(currentChildNode);
                visited.insert(currentChildNode);
            }
        }
    }
    
    return false;
}