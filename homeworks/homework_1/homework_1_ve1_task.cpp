#include <vector>
#include <string>
#include <iostream>
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

std::vector<std::pair<size_t, size_t>> generateNeighbourMoves(const StateNode& gameStateNode) {
    std::vector<std::pair<size_t, size_t>> resultMoves;
    size_t separatorIndex = gameStateNode.separatorIndex;

    // Slide: right-facing frog moves one position forward into the empty slot
    if (separatorIndex > 0 &&
        gameStateNode.dataState[separatorIndex - 1] == CONSTANTS::RIGHT_ARROW) {
        resultMoves.emplace_back(separatorIndex, separatorIndex - 1);
    }

    // Jump: right-facing frog leaps two positions over the opposite-facing frog in between
    if (separatorIndex >= 2 &&
        gameStateNode.dataState[separatorIndex - 2] == CONSTANTS::RIGHT_ARROW &&
        gameStateNode.dataState[separatorIndex - 1] == CONSTANTS::LEFT_ARROW) {
        resultMoves.emplace_back(separatorIndex, separatorIndex - 2);
    }

    // Slide: left-facing frog moves one position forward into the empty slot
    if (separatorIndex < gameStateNode.dataState.size() - 1 &&
        gameStateNode.dataState[separatorIndex + 1] == CONSTANTS::LEFT_ARROW) {
        resultMoves.emplace_back(separatorIndex, separatorIndex + 1);
    }

    // Jump: left-facing frog leaps two positions over the opposite-facing frog in between
    if (separatorIndex < gameStateNode.dataState.size() - 2 &&
        gameStateNode.dataState[separatorIndex + 2] == CONSTANTS::LEFT_ARROW &&
        gameStateNode.dataState[separatorIndex + 1] == CONSTANTS::RIGHT_ARROW) {
        resultMoves.emplace_back(separatorIndex, separatorIndex + 2);
    }

    return resultMoves;
}

bool dfs(std::unordered_set<std::string>& visited, std::vector<std::string>& nodePath,
    StateNode& currentNode, const StateNode& goalNode) {
    // Recording the state snapshot before exploring its children so backtracking can undo it
    std::string currentSnapshot = currentNode.dataState;
    nodePath.push_back(currentSnapshot);
    if (currentNode.dataState == goalNode.dataState) {
        return true;
    }

    visited.insert(currentSnapshot);
    for (const auto& [a, b] : generateNeighbourMoves(currentNode)) {
        currentNode.modify(a, b);
        if (visited.find(currentNode.dataState) == visited.end()) {
            if (dfs(visited, nodePath, currentNode, goalNode)) {
                return true;
            }
        }
        // Undo the move — same swap restores the previous state
        currentNode.modify(a, b);
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
    std::unordered_set<std::string> visited;
    std::vector<std::string> nodePath;

    // Executing the modified dfs algorithm
    if (dfs(visited, nodePath, startStateNode, endStateNode)) {
        for (const std::string& snapshot : nodePath) {
            std::cout << snapshot << '\n';
        }
    }
}

#ifndef PROFILING_MODE
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    solveTask();
    return 0;
}
#endif