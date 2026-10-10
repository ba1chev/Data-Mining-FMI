#include <queue>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

void bfs(std::unordered_map<int, std::unordered_set<int>>& graph, const int start) {
    std::unordered_set<int> visited;
    std::queue<int> q;

    q.push(start);
    visited.insert(start);
    std::cout << start << std::endl;
    while (!q.empty()) {
        const int current = q.front();
        q.pop();

        for (const int currentChild: graph[current]) {
            if (visited.find(currentChild) == visited.end()) {
                q.push(currentChild);
                visited.insert(currentChild);
                std::cout << currentChild << " ";
            }
        }
        std::cout << std::endl;
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
    bfs(graph, 0);

    return 0;
}