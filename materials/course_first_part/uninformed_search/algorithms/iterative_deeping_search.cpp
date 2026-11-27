#include <iostream>
#include <unordered_map>
#include <unordered_set>

bool dls(std::unordered_map<int, std::unordered_set<int>>& graph,
    const int currentNode, const int goalNode,
    const int currentDepth, const int depthTolerance,
    std::unordered_map<int, int>& bestDepth) {
    if (currentNode == goalNode) {
        return true;
    }
    if (currentDepth == depthTolerance) {
        return false;
    }

    bestDepth[currentNode] = currentDepth;
    for (const int currentChildNode: graph[currentNode]) {
        const auto& currentChildNodeIter = bestDepth.find(currentChildNode);
        if (currentChildNodeIter == bestDepth.end() || 
            currentChildNodeIter->second > currentDepth + 1) {
            if (dls(graph, currentChildNode, goalNode, 
                currentDepth + 1, depthTolerance, bestDepth)) {
                return true;    
            }
        }
    }

    return false;
}

bool ids(std::unordered_map<int, std::unordered_set<int>>& graph,
    const int startNode, const int goalNode, const int upperBound) {
    std::unordered_map<int, int> bestDepth;
    for (size_t i = 0; i < upperBound; i++) {    
        if (dls(graph, startNode, goalNode, 0, i, bestDepth)) {
            return true;
        }
        bestDepth.clear();
    }
    
    return false;
}