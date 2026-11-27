#include <vector>
#include <climits>
#include <iostream>
#include <algorithm>

struct Node {
public:
    std::vector<Node*> children;
    int value = 0;

    Node(const int value) {
        this->value = value;
    }

    Node(const std::vector<Node*>& children) {
        this->children = children;
    }

    ~Node() {
        for (Node* currentChild: children) {
            delete currentChild;
        }
    }
};

int minimax(const Node* currentNode, const bool isMaximizing) {
    if (currentNode->children.empty()) {
        return currentNode->value;
    }

    int bestValue = isMaximizing ? INT_MIN : INT_MAX;
    for (const Node* currentChild: currentNode->children) {
        int currentValue = minimax(currentChild, !isMaximizing);
        bestValue = isMaximizing ? std::max(bestValue, currentValue) : std::min(bestValue, currentValue);
    }
    return bestValue;
}

int bestMove(const Node* root, const bool isMaximizing) {
    int bestIndex = -1;
    int bestValue = isMaximizing ? INT_MIN : INT_MAX;
    for (size_t i = 0; i < root->children.size(); i++) {
        int currentValue = minimax(root->children[i], !isMaximizing);
        if (isMaximizing ? currentValue > bestValue : currentValue < bestValue) {
            bestValue = currentValue;
            bestIndex = i;
        }
    }
    return bestIndex;
}
