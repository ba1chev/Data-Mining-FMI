#include <iostream>
#include <unordered_map>
#include <unordered_set>

bool dls(std::unordered_map<int, std::unordered_set<int>>& graph,
    std::unordered_map<int, int>& bestDistance, const int currentNode, const int goalNode,
    const int tolerance, const int counter) {
    if (currentNode == goalNode) {
        return true;
    }
    if (counter == tolerance) {
        return false;
    }

    const auto& currentIterator = bestDistance.find(currentNode);
    if (currentIterator != bestDistance.end() && 
        currentIterator->second <= counter) {
        return false;
    }

    bestDistance[currentNode] = counter;
    for (const int currentChildNode: graph[currentNode]) {
        if (dls(graph, bestDistance, currentChildNode, 
            goalNode, tolerance, counter + 1)) {
            return true;
        }
    }

    return false;
}