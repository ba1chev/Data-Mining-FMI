#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

typedef int (*HeuriticsFunction)(const int&);

struct Edge {
public:
    int to = 0;
    int weight = 0;

    Edge(const int to, const int weight) {
        this->to = to;
        this->weight = weight;
    } 
};

bool idaStarRecursive(std::unordered_map<int, std::vector<Edge>>& graph,
    std::unordered_map<int, int>& bestDistance, const int currentNode, const int goalNode, 
    HeuriticsFunction heuriticsFunction, const int distance,
    const int tolerance, int& newTolerance) {
    int currentDistance = distance + heuriticsFunction(currentNode);
    if (currentDistance > tolerance) {
        newTolerance = std::min(newTolerance, currentDistance);
        return false;
    }
    if (currentNode == goalNode) {
        return true;
    }

    const auto& currentIterator = bestDistance.find(currentNode);
    if (currentIterator != bestDistance.end() &&
        currentIterator->second <= distance) {
        return false;
    }

    bestDistance[currentNode] = distance;
    for (const Edge& currentChildNode: graph[currentNode]) {
        if (idaStarRecursive(graph, bestDistance, currentChildNode.to, 
            goalNode, heuriticsFunction, distance + currentChildNode.weight,
            tolerance, newTolerance)) {
            return true;
        }
    }

    return false;
}

int idaStarSearch(std::unordered_map<int, std::vector<Edge>>& graph,
    const int startNode, const int goalNode, HeuriticsFunction heuristicsFunction) {
    std::unordered_map<int, int> bestDistance;
    int tolerance = heuristicsFunction(startNode);

    while (true) {
        bestDistance.clear();
        int nextTolerance = INT_MAX;

        if (idaStarRecursive(graph, bestDistance, startNode, goalNode,
            heuristicsFunction, 0, tolerance, nextTolerance)) {
            return tolerance;
        }
        if (nextTolerance == INT_MAX) {
            return -1;
        }
        tolerance = nextTolerance;
    }
}