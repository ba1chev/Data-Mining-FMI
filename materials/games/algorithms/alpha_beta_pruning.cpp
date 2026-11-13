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

int alphaBeta(const Node* currentNode, int alpha, int beta, const bool isMaximizing) {
    if (currentNode->children.empty()) {
        return currentNode->value;
    }

    int bestValue = isMaximizing ? INT_MIN : INT_MAX;
    for (const Node* currentChild: currentNode->children) {
        int currentValue = alphaBeta(currentChild, alpha, beta, !isMaximizing);
        if (isMaximizing) {
            bestValue = std::max(bestValue, currentValue);
            alpha = std::max(alpha, bestValue);
        } else {
            bestValue = std::min(bestValue, currentValue);
            beta = std::min(beta, bestValue);
        }

        if (beta <= alpha) {
            break;
        }
    }
    return bestValue;
}

int bestMove(const Node* root, const bool isMaximizing) {
    int bestIndex = -1;
    int alpha = INT_MIN;
    int beta = INT_MAX;
    for (size_t i = 0; i < root->children.size(); i++) {
        int currentValue = alphaBeta(root->children[i], alpha, beta, !isMaximizing);
        if (isMaximizing && currentValue > alpha) {
            alpha = currentValue;
            bestIndex = i;
        } else if (!isMaximizing && currentValue < beta) {
            beta = currentValue;
            bestIndex = i;
        }
    }
    return bestIndex;
}
