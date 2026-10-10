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
    std::vector<int> distances(countOfNodes, INT_MAX);
    std::vector<bool> visited(countOfNodes, false);
    std::vector<int> parents(countOfNodes, -1);
    std::priority_queue<Node> q;

    distances[startNode] = 0;
    q.push(Node(startNode, 0));
    while (!q.empty()) {
        Node currentNode = q.top();
        q.pop();

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

        for (const Edge& currentChild: graph[currentNode.node]) {
            int currentDistance = currentNode.distance + currentChild.weight;
            if (currentDistance < distances[currentChild.to]) {
                q.push(Node(currentChild.to, currentDistance));
                distances[currentChild.to] = currentDistance;
                parents[currentChild.to] = currentNode.node;
            }
        }
    }

    return {};
}

int main() {
    std::unordered_map<int, std::vector<Edge>> graph;
    graph[0].push_back({1, 1});
    graph[0].push_back({2, 4});
    graph[1].push_back({3, 2});
    graph[1].push_back({4, 5});
    graph[2].push_back({3, 1});
    graph[3].push_back({4, 1});

    const int start = 0, end = 4, nodes = 5;
    std::vector<int> path = ucs(graph, start, end, nodes);

    if (path.empty()) {
        std::cout << "No path found from " << start << " to " << end << "\n";
    } else {
        std::cout << "Shortest path from " << start << " to " << end << ": ";
        for (int i = 0; i < (int)path.size(); ++i) {
            if (i) std::cout << " -> ";
            std::cout << path[i];
        }
        std::cout << "\n";
    }

    return 0;
}