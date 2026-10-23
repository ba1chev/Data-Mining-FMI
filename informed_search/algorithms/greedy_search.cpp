#include <queue>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

typedef int (*Heuristics)(const int&);

struct Node {
public:
    int node = 0;
    int distance = 0;
    int heuristics = 0;

    Node(const int node, const int distance,
        const int heuristics) {
        this->node = node;
        this->distance = distance;
        this->heuristics = heuristics;
    }

    bool operator < (const Node& other) const {
        return this->heuristics > other.heuristics;
    }
};

struct Edge {
public:
    int to = 0;    
    int weight = 0;

    Edge(const int to, const int weight) {
        this->to = to;
        this->weight = weight;
    }
};

std::vector<int> greedySearch(std::unordered_map<int, std::vector<Edge>>& graph,
    const int startNode, const int goalNode,
    Heuristics heuristics, const int countOfNodes) {
    if (startNode == goalNode) {
        std::vector<int> result;
        result.push_back(startNode);
        return result;
    }

    std::priority_queue<Node> pq;
    std::unordered_set<int> visited;
    std::vector<int> parents(countOfNodes, -1);
    std::vector<int> distances(countOfNodes, INT_MAX);
    
    pq.push(Node(startNode, 0, heuristics(startNode)));
    while (!pq.empty()) {
        Node currentNode = pq.top();
        pq.pop();

        if (currentNode.node == goalNode) {
            std::vector<int> result;
            for (int i = goalNode; i != -1; i = parents[i]) {
                result.push_back(i);
            }
            std::reverse(result.begin(), result.end());
            return result;
        }

        if (visited.find(currentNode.node) != visited.end()) {
            continue;
        }
        visited.insert(currentNode.node);
        
        for (const Edge& currentChildNode: graph[currentNode.node]) {
            if (visited.find(currentChildNode.to) != visited.end()) {
                continue;
            }

            parents[currentChildNode.to] = currentNode.node;
            int currentDistance = currentNode.distance + currentChildNode.weight;
            pq.push(Node(currentChildNode.to, currentDistance, heuristics(currentChildNode.to)));
        }
    }
    return {};
}