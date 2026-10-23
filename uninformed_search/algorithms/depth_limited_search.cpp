#include <iostream>
#include <unordered_map>
#include <unordered_set>

bool dls(std::unordered_map<int, std::unordered_set<int>>& graph,
    std::unordered_map<int, int>& bestDepth, const int currentNode, 
    const int tolerance, const int counter) {
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
        if (dls(graph, bestDepth, currentChild, tolerance, counter + 1)) {
            return true;
        }
    }
    return false;
}