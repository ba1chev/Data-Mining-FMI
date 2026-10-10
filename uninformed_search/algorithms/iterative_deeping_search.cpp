#include <iostream>
#include <unordered_map>
#include <unordered_set>

bool dls(std::unordered_map<int, std::unordered_set<int>>& graph,
    std::unordered_map<int, int>& bestDepth, const int currentNode,
    const int goalNode, const int tolerance, const int counter) {
    if (currentNode == goalNode) {
        return true;
    }
    if (counter == tolerance) {
        return false;
    }

    const auto& currentIterator = bestDepth.find(currentNode);
    if (currentIterator != bestDepth.end() &&
        currentIterator->second <= counter) {
        return false;
    }

    bestDepth[currentNode] = counter;
    for (const int currentChild: graph[currentNode]) {
        if (dls(graph, bestDepth, currentChild, goalNode, tolerance, counter + 1)) {
            return true;
        }
    }
    return false;
}

bool ids(std::unordered_map<int, std::unordered_set<int>>& graph,
    const int startNode, const int goalNode, const int upperBound) {
    for (size_t i = 0; i < upperBound; i++) {
        std::unordered_map<int, int> bestDepth;
        if (dls(graph, bestDepth, startNode, goalNode, i, 0)) {
            return true;
        }
    }
    return false;
}

int main() {
    std::unordered_map<int, std::unordered_set<int>> graph;
    graph[0].insert(1);
    graph[0].insert(2);
    graph[1].insert(3);
    graph[1].insert(4);
    graph[2].insert(5);

    const int goalNode = 4;
    const bool found = ids(graph, 0, goalNode, 5);
    std::cout << (found ? "Goal found" : "Goal not found") << "\n";

    return 0;
}