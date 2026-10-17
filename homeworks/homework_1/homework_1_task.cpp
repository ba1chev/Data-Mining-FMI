#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

enum class StateNodeType: uint8_t {
    START, END, MIDDLE
};

struct StateNode {
public:
    size_t separatorIndex = 0;
    std::string dataState = "";
    StateNodeType type = StateNodeType::START;

    size_t stateNodeId = 0;
    // Numeric IDs are used as graph keys instead of string data to avoid hashing overhead
    static size_t idMask;

    StateNode() = default;
    StateNode(const std::string& dataState, 
        const size_t separatorIndex, StateNodeType type) {
        this->type = type;
        this->dataState = dataState;
        this->stateNodeId = idMask;
        this->separatorIndex = separatorIndex;
        idMask += 1;
    }

    void modify(size_t leftIndex, size_t rightIndex) {
        if (leftIndex > rightIndex) {
            std::swap(leftIndex, rightIndex);
        }
        if (rightIndex >= dataState.size()) {
            throw std::out_of_range("Invalid position.");
        }
        // Keeping track of where the separator ends up after the swap
        if (separatorIndex == leftIndex) {
            separatorIndex = rightIndex;
        }
        else if (separatorIndex == rightIndex) {
            separatorIndex = leftIndex;
        }
        std::swap(dataState[leftIndex], dataState[rightIndex]);
    }

    friend std::ostream& operator << (std::ostream& os, const StateNode& stateNode) {
        os << stateNode.dataState;
        return os;
    }
};
size_t StateNode::idMask = 0;

namespace CONSTANTS {
    const static char LEFT_ARROW = '<';
    const static char RIGHT_ARROW = '>';
    const static char SEPARATOR = '_';
}

namespace BUILD {
    StateNode buildNode(const size_t N, StateNodeType type) {
        std::string resultNodeState = "";
        switch (type) {
            case StateNodeType::START: {
                for (size_t i = 0; i < N; i++) {
                    resultNodeState += CONSTANTS::RIGHT_ARROW;
                }
                resultNodeState += CONSTANTS::SEPARATOR;
                for (size_t i = 0; i < N; i++) {
                    resultNodeState += CONSTANTS::LEFT_ARROW;
                }
                break;
            }
            case StateNodeType::END: {
                for (size_t i = 0; i < N; i++) {
                    resultNodeState += CONSTANTS::LEFT_ARROW;
                }
                resultNodeState += CONSTANTS::SEPARATOR;
                for (size_t i = 0; i < N; i++) {
                    resultNodeState += CONSTANTS::RIGHT_ARROW;
                }
                break;
            }
            default: {
                throw std::runtime_error("Invalid input data");
            }
        }

        return StateNode(resultNodeState, N, type);
    }
};

std::vector<StateNode> generateNeighbourStates(const StateNode& gameStateNode) {
    std::vector<StateNode> resultNeighboursNodes;
    StateNode currentStateNode;
    size_t separatorIndex = gameStateNode.separatorIndex;

    // Slide: right-facing frog moves one position forward into the empty slot
    if (separatorIndex > 0 &&
        gameStateNode.dataState[separatorIndex - 1] == CONSTANTS::RIGHT_ARROW) {
        currentStateNode = gameStateNode;
        currentStateNode.stateNodeId = StateNode::idMask;
        StateNode::idMask += 1;
        currentStateNode.modify(separatorIndex, separatorIndex - 1);
        resultNeighboursNodes.push_back(currentStateNode);
    }

    // Jump: right-facing frog leaps two positions over the opposite-facing frog in between
    if (separatorIndex >= 2 &&
        gameStateNode.dataState[separatorIndex - 2] == CONSTANTS::RIGHT_ARROW &&
        gameStateNode.dataState[separatorIndex - 1] == CONSTANTS::LEFT_ARROW) {
        currentStateNode = gameStateNode;
        currentStateNode.stateNodeId = StateNode::idMask;
        StateNode::idMask += 1;
        currentStateNode.modify(separatorIndex, separatorIndex - 2);
        resultNeighboursNodes.push_back(currentStateNode);
    }

    // Slide: left-facing frog moves one position forward into the empty slot
    if (separatorIndex < gameStateNode.dataState.size() - 1 &&
        gameStateNode.dataState[separatorIndex + 1] == CONSTANTS::LEFT_ARROW) {
        currentStateNode = gameStateNode;
        currentStateNode.stateNodeId = StateNode::idMask;
        StateNode::idMask += 1;
        currentStateNode.modify(separatorIndex, separatorIndex + 1);
        resultNeighboursNodes.push_back(currentStateNode);
    }

    // Jump: left-facing frog leaps two positions over the opposite-facing frog in between
    if (separatorIndex < gameStateNode.dataState.size() - 2 &&
        gameStateNode.dataState[separatorIndex + 2] == CONSTANTS::LEFT_ARROW &&
        gameStateNode.dataState[separatorIndex + 1] == CONSTANTS::RIGHT_ARROW) {
        currentStateNode = gameStateNode;
        currentStateNode.stateNodeId = StateNode::idMask;
        StateNode::idMask += 1;
        currentStateNode.modify(separatorIndex, separatorIndex + 2);
        resultNeighboursNodes.push_back(currentStateNode);
    }

    return resultNeighboursNodes;
}

bool dfs(std::unordered_map<size_t, std::unordered_set<size_t>>& graph,
    std::unordered_set<std::string>& visited, std::vector<StateNode>& nodePath,
    const StateNode& currentNode, const StateNode& goalNode) {
    // Recording the state before exploring its children so backtracking can undo it
    nodePath.push_back(currentNode);
    if (currentNode.dataState == goalNode.dataState) {
        return true;
    }

    visited.insert(currentNode.dataState);
    std::vector<StateNode> neighbours = generateNeighbourStates(currentNode);
    for (const StateNode& childNode: neighbours) {
        graph[currentNode.stateNodeId].insert(childNode.stateNodeId);
        if (visited.find(childNode.dataState) != visited.end()) {
            continue;
        }
        if (dfs(graph, visited, nodePath, childNode, goalNode)) {
            return true;
        }
    }

    nodePath.pop_back();
    return false;
}

void solveTask() {
    // Entering input data
    size_t N = 0;
    std::cin >> N;

    // Defining start and end state of the program execution
    StateNode startStateNode = BUILD::buildNode(N, StateNodeType::START);
    StateNode endStateNode = BUILD::buildNode(N, StateNodeType::END);
    
    // Defining data structures used by the DFS search
    std::unordered_map<size_t, std::unordered_set<size_t>> graph;
    std::unordered_set<std::string> visited;
    std::vector<StateNode> nodePath;

    // Executing the modified dfs algorithm
    if (dfs(graph, visited, nodePath, startStateNode, endStateNode)) {
        for (const StateNode& currentStateNode: nodePath) {
            std::cout << currentStateNode << std::endl;
        }
    }
}

#ifndef PROFILING_MODE
int main() {
    solveTask();
    return 0;
}
#endif