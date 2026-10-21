#include <queue>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

typedef int (*Heuristics)(int);

struct Node {
public:
    int node = 0;
    int distance = 0;
    int heuristicsValue = 0;

    Node(const int node, const int distance, const int heuristicsValue) {
        this->node = node;
        this->distance = distance;
        this->heuristicsValue = heuristicsValue;
    }

    bool operator < (const Node& other) const {
        return this->heuristicsValue > other.heuristicsValue;
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
    if (!heuristics) {
        throw std::runtime_error("Nullptr detected");
    }
    if (startNode == goalNode) {
        std::vector<int> result;
        result.push_back(startNode);
        return result;
    }

    std::priority_queue<Node> pq;
    std::vector<bool> visited(countOfNodes, false);
    std::vector<int> distances(countOfNodes, INT_MAX);
    std::vector<int> parents(countOfNodes, -1);

    pq.push(Node(startNode, 0, heuristics(startNode)));
    distances[startNode] = heuristics(startNode);
    while (!pq.empty()) {
        Node currentNode = pq.top();
        pq.pop();

        if (visited[currentNode.node]) {
            continue;
        }
        visited[currentNode.node] = true;

        if (currentNode.node == goalNode) {
            std::vector<int> result;
            for (int i = goalNode; i != -1; i = parents[i]) {
                result.push_back(i);
            }
            std::reverse(result.begin(), result.end());
            return result;
        }
        
        for (const Edge& currentChildNode: graph[currentNode.node]) {
            int currentDistance = heuristics(currentNode.node) + heuristics(currentChildNode.to);
            if (currentDistance < distances[currentChildNode.to]) {
                pq.push(Node(currentChildNode.to, currentDistance, heuristics(currentChildNode.to)));
                distances[currentChildNode.to] = heuristics(currentChildNode.to);
                parents[currentChildNode.to] = currentNode.node;
            }
        }
    }

    return {};
}

const int H[] = {6, 4, 2, 5, 0, 3};

int heuristicsToG(int node) {
    return H[node];
}

int main() {
    const int countOfNodes = 6;
    std::unordered_map<int, std::vector<Edge>> graph = {
        {0, {{1, 1}, {3, 2}}},
        {1, {{2, 5}}},
        {2, {{4, 9}}},
        {3, {{4, 1}}}
    };

    std::vector<int> path = greedySearch(graph, 0, 4, heuristicsToG, countOfNodes);
    for (int node : path) std::cout << node << ' ';
    std::cout << std::endl;

    path = greedySearch(graph, 4, 4, heuristicsToG, countOfNodes);
    for (int node : path) std::cout << node << ' ';
    std::cout << std::endl;

    path = greedySearch(graph, 0, 5, heuristicsToG, countOfNodes);
    std::cout << (path.empty() ? "no path" : "found") << std::endl;

    greedySearch(graph, 0, 4, nullptr, countOfNodes);
    return 0;
}