#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

// All four possible moves:
// 1) '>_'  goes to '_>'
// 2) '_<'  goes to '<_'
// 3) '>*_' goes to '_*>'
// 4) '_*<' goes to '<*_'
std::unordered_map<std::string, std::string> casesMatching {
    {">_",  "_>"},
    {"_<",  "<_"},
    {">>_", "_>>"}, {"><_", "_<>"},
    {"_<<", "<<_"}, {"_><", "<>_"}
};

std::pair<std::string, std::string> buildStartFinalStates(const size_t N) {
    std::pair<std::string, std::string> startEndStates;
    startEndStates.first = "";
    startEndStates.second = "";

    for (size_t i = 0; i < N; i++) {
        startEndStates.first += ">";
        startEndStates.second += "<";
    }
    startEndStates.first += "_";
    startEndStates.second += "_";
    for (size_t i = 0; i < N; i++) {
        startEndStates.first += "<";
        startEndStates.second += ">";
    }

    return startEndStates;
}

std::vector<std::string> generateStateChildStates(const std::string& currentState) {
    std::vector<std::string> stateChildStates;
    size_t separatorIndex = currentState.find('_');

    // A frog can move into the empty cell from:
    // 1) separatorIndex + 1 if it is '<'
    // 2) separatorIndex - 1 if it is '>'
    // 3) separatorIndex + 2 if it is '<'
    // 4) separatorIndex - 2 if it is '>'
    std::string currentBranchState = currentState;
    if (separatorIndex < currentState.size() - 1 && currentBranchState[separatorIndex + 1] == '<') {
        std::swap(currentBranchState[separatorIndex], currentBranchState[separatorIndex + 1]);
        stateChildStates.push_back(currentBranchState);
        currentBranchState = currentState;
    }
    if (separatorIndex > 0 && currentBranchState[separatorIndex - 1] == '>') {
        std::swap(currentBranchState[separatorIndex - 1], currentBranchState[separatorIndex]);
        stateChildStates.push_back(currentBranchState);
        currentBranchState = currentState;
    }
    if (separatorIndex < currentState.size() - 2 && currentBranchState[separatorIndex + 2] == '<') {
        std::swap(currentBranchState[separatorIndex], currentBranchState[separatorIndex + 2]);
        stateChildStates.push_back(currentBranchState);
        currentBranchState = currentState;
    }
    if (separatorIndex >= 2 && currentBranchState[separatorIndex - 2] == '>') {
        std::swap(currentBranchState[separatorIndex - 2], currentBranchState[separatorIndex]);
        stateChildStates.push_back(currentBranchState);
        currentBranchState = currentState;
    }

    return stateChildStates;
}

// States from which the final state can no longer be reached:
// 1) '>><<' none of these frogs can ever move again
// 2) '<<_' with '>' on the left  the '>' frogs on the left are stuck
// 3) '_>>' with '<' on the right the '<' frogs on the right are stuck
bool isDeadState(const std::string& currentState) {
    if (currentState.find(">><<") != std::string::npos) {
        return true;
    }
    size_t separatorIndex = currentState.find('_');
    size_t currentStateSize = currentState.size();

    if (separatorIndex >= 2 && currentState[separatorIndex - 1] == '<' &&
        currentState[separatorIndex - 2] == '<' &&
        currentState.find('>') < separatorIndex)
        return true;
    if (separatorIndex + 2 < currentStateSize && currentState[separatorIndex + 1] == '>' &&
        currentState[separatorIndex + 2] == '>' &&
        currentState.rfind('<') > separatorIndex)
        return true;

    return false;
}

bool dfs(const std::string& currentState, const std::string& finalState,
    std::vector<std::string>& statesPath, std::unordered_set<std::string>& visited) {
    if (currentState == finalState) {
        statesPath.push_back(finalState);
        return true;
    }

    visited.insert(currentState);
    statesPath.push_back(currentState);
    std::vector<std::string> currentStateChildStates = generateStateChildStates(currentState);
    for (const std::string& currentStateChildState: currentStateChildStates) {
        if (visited.find(currentStateChildState) == visited.end() &&
            !isDeadState(currentStateChildState)) {
            if (dfs(currentStateChildState, finalState, statesPath, visited)) {
                return true;
            }
        }
    }

    // Dead end, backtrack
    statesPath.pop_back();
    return false;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t N = 0;
    std::cin >> N;
    std::vector<std::string> statesPath;
    std::unordered_set<std::string> visitedStates;
    std::pair<std::string, std::string> startFinalStates = buildStartFinalStates(N);

    dfs(startFinalStates.first, startFinalStates.second, statesPath, visitedStates);

    for (const std::string& currentState: statesPath) {
        std::cout << currentState << std::endl;
    }

    return 0;
}