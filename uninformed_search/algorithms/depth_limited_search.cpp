#include <iostream>
#include <unordered_map>
#include <unordered_set>

void dls(std::unordered_map<int, std::unordered_set<int>>& graph,
    std::unordered_map<int, int>& bestDepth, const int currentNode, 
    const int tolerance, const int counter) {
    if (counter == tolerance) {
        return;
    }

    const auto iterator = bestDepth.find(currentNode);
    if (iterator != bestDepth.end() && iterator->second <= counter) {
        return;
    }

    bestDepth[currentNode] = counter;
    for (const int currentChild: graph[currentNode]) {
        dls(graph, bestDepth, currentChild, tolerance, counter + 1);
    }
}

int main() {
    std::unordered_map<int, std::unordered_set<int>> graph;
    graph[0].insert(1);
    graph[0].insert(2);
    graph[1].insert(3);
    graph[1].insert(4);
    graph[2].insert(5);

    std::unordered_map<int, int> bestDepth;
    const int tolerance = 2;
    dls(graph, bestDepth, 0, tolerance, 0);

    for (const auto& [node, depth] : bestDepth) {
        std::cout << "Node " << node << " reached at depth " << depth << "\n";
    }

    return 0;
}