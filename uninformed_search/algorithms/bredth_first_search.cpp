#include <queue>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

bool bfs(std::unordered_map<int, std::unordered_set<int>>& graph, 
    const int startNode, const int goalNode) {
    if (startNode == goalNode) {
        return true;
    }

    std::unordered_set<int> visited;
    std::queue<int> q;

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

int main() {
    std::unordered_map<int, std::unordered_set<int>> graph;
    for (size_t i = 0; i < 4; i++) {
        graph[i];
    }
    
    graph[0].insert(1);
    graph[0].insert(2);
    graph[0].insert(3);
    bfs(graph, 0, 3);

    return 0;
}