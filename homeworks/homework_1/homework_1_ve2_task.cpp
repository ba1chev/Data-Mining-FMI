#include <vector>
#include <string>
#include <iostream>
#include <utility>

enum class StateNodeType: uint8_t {
    START, END
};

enum class Direction: uint8_t {
    RIGHT, LEFT
};

Direction opposite(Direction direction) {
    return direction == Direction::RIGHT ? Direction::LEFT : Direction::RIGHT;
}

struct StateNode {
public:
    size_t separatorIndex = 0;
    std::string dataState = "";
    StateNodeType type = StateNodeType::START;

    StateNode() = default;
    StateNode(const std::string& dataState,
        const size_t separatorIndex, StateNodeType type): 
        dataState(dataState), separatorIndex(separatorIndex), type(type) {}

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

namespace CONSTANTS {
    const static char LEFT_ARROW = '<';
    const static char RIGHT_ARROW = '>';
    const static char SEPARATOR = '_';
}

namespace BUILD {
    StateNode buildNode(const size_t N, StateNodeType type) {
        std::string resultNodeState;
        switch (type) {
            case StateNodeType::START: {
                resultNodeState.append(N, CONSTANTS::RIGHT_ARROW);
                resultNodeState += CONSTANTS::SEPARATOR;
                resultNodeState.append(N, CONSTANTS::LEFT_ARROW);
                break;
            }
            case StateNodeType::END: {
                resultNodeState.append(N, CONSTANTS::LEFT_ARROW);
                resultNodeState += CONSTANTS::SEPARATOR;
                resultNodeState.append(N, CONSTANTS::RIGHT_ARROW);
                break;
            }
            default: {
                throw std::runtime_error("Invalid input data");
            }
        }
        return StateNode(resultNodeState, N, type);
    }
}

class RuleBasedSolver {
    StateNode stateNode;
    size_t N;

    std::vector<std::pair<Direction, size_t>> generateMoveGroups() const {
        std::vector<std::pair<Direction, size_t>> groups;
        Direction direction = Direction::RIGHT;

        auto addGroup = [&](size_t count) {
            groups.emplace_back(direction, count);
            direction = opposite(direction);
        };

        // Pattern: [1, 2, ..., N, N, N, N-1, ..., 1] with alternating directions
        for (size_t i = 1; i <= N; i++) addGroup(i);
        addGroup(N);
        addGroup(N);
        for (size_t i = N; i > 1; i--) addGroup(i - 1);

        return groups;
    }

    void applyNextMove(Direction direction) {
        size_t blank = stateNode.separatorIndex;
        size_t size = stateNode.dataState.size();

        if (direction == Direction::RIGHT) {
            // Jump: right-facing frog leaps two positions over the opposite-facing frog in between
            if (blank >= 2 &&
                stateNode.dataState[blank - 2] == CONSTANTS::RIGHT_ARROW &&
                stateNode.dataState[blank - 1] == CONSTANTS::LEFT_ARROW) {
                stateNode.modify(blank, blank - 2);
            }
            // Slide: right-facing frog moves one position forward into the empty slot
            else {
                stateNode.modify(blank, blank - 1);
            }
        }
        else {
            // Jump: left-facing frog leaps two positions over the opposite-facing frog in between
            if (blank <= size - 3 &&
                stateNode.dataState[blank + 2] == CONSTANTS::LEFT_ARROW &&
                stateNode.dataState[blank + 1] == CONSTANTS::RIGHT_ARROW) {
                stateNode.modify(blank, blank + 2);
            }
            // Slide: left-facing frog moves one position forward into the empty slot
            else {
                stateNode.modify(blank, blank + 1);
            }
        }
    }

public:
    RuleBasedSolver(StateNode stateNode, size_t N): 
        stateNode(std::move(stateNode)), N(N) {}

    void solve(std::ostream& out) {
        out << stateNode << '\n';
        for (const auto& [direction, count] : generateMoveGroups()) {
            for (size_t i = 0; i < count; i++) {
                applyNextMove(direction);
                out << stateNode << '\n';
            }
        }
    }
};

void solveTask() {
    size_t N = 0;
    std::cin >> N;

    StateNode startStateNode = BUILD::buildNode(N, StateNodeType::START);
    RuleBasedSolver solver(std::move(startStateNode), N);
    solver.solve(std::cout);
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    solveTask();
    return 0;
}
