#include <queue>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

struct Node {
public:
    int node = 0;
    int distance = 0;

    Node(const int node, const int distance) {
        this->node = node;
        this->distance = distance;
    }

    bool operator < (const Node& other) const {
        return this->distance > other.distance;
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

std::vector<int> ucs(std::unordered_map<int, std::vector<Edge>>& graph,
    const int startNode, const int goalNode, const int countOfNodes) {
    std::priority_queue<Node> q;
    std::unordered_set<int> visited;
    std::vector<int> parents(countOfNodes, -1);
    std::vector<int> distances(countOfNodes, INT_MAX);
        
    distances[startNode] = 0;
    q.push(Node(startNode, 0));
    while (!q.empty()) {
        Node currentNode = q.top();
        q.pop();

        if (visited.find(currentNode.node) != visited.end()) {
            continue;
        }
        visited.insert(currentNode.node);

        if (currentNode.node == goalNode) {
            std::vector<int> result;
            for (int i = goalNode; i != -1; i = parents[i]) {
                result.push_back(i);
            }
            std::reverse(result.begin(), result.end());
            return result;
        }

        for (const Edge& currentChildNode: graph[currentNode.node]) {
            if (visited.find(currentChildNode.to) != visited.end()) {
                continue;
            }

            int currentDistance = currentNode.distance + currentChildNode.weight;
            if (currentDistance < distances[currentChildNode.to]) {
                q.push(Node(currentChildNode.to, currentDistance));
                distances[currentChildNode.to] = currentDistance;
                parents[currentChildNode.to] = currentNode.node;
            }
        }
    }

    return {};
}