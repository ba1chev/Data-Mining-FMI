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
    std::cout << startNode << std::endl;
    while (!q.empty()) {
        const int current = q.front();
        q.pop();
        if (current == goalNode) {
            return true;
        }

        for (const int currentChild: graph[current]) {
            if (visited.find(currentChild) == visited.end()) {
                q.push(currentChild);
                visited.insert(currentChild);
            }
        }
    }
    
    return false;
}