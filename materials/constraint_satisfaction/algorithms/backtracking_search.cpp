#include <vector>
#include <iostream>
#include <unordered_map>

typedef bool (*ConstraintFunction)(const int, const int);

struct Constraint {
public:
    int first = 0;
    int second = 0;
    ConstraintFunction isSatisfied = nullptr;

    Constraint(const int first, const int second, ConstraintFunction isSatisfied) {
        this->first = first;
        this->second = second;
        this->isSatisfied = isSatisfied;
    }
};

bool isConsistent(const std::unordered_map<int, int>& assignment,
    const std::vector<Constraint>& constraints, const int variable, const int value) {
    for (const Constraint& currentConstraint: constraints) {
        if (currentConstraint.first == variable) {
            const auto& currentIterator = assignment.find(currentConstraint.second);
            if (currentIterator != assignment.end() &&
                !currentConstraint.isSatisfied(value, currentIterator->second)) {
                return false;
            }
        } else if (currentConstraint.second == variable) {
            const auto& currentIterator = assignment.find(currentConstraint.first);
            if (currentIterator != assignment.end() &&
                !currentConstraint.isSatisfied(currentIterator->second, value)) {
                return false;
            }
        }
    }
    return true;
}

bool backtrack(const std::vector<int>& variables,
    std::unordered_map<int, std::vector<int>>& domains,
    const std::vector<Constraint>& constraints,
    std::unordered_map<int, int>& assignment, const size_t variableIndex) {
    if (variableIndex == variables.size()) {
        return true;
    }

    const int currentVariable = variables[variableIndex];
    for (const int currentValue: domains[currentVariable]) {
        if (isConsistent(assignment, constraints, currentVariable, currentValue)) {
            assignment[currentVariable] = currentValue;
            if (backtrack(variables, domains, constraints, assignment, variableIndex + 1)) {
                return true;
            }
            assignment.erase(currentVariable);
        }
    }
    return false;
}

std::unordered_map<int, int> backtrackingSearch(const std::vector<int>& variables,
    std::unordered_map<int, std::vector<int>>& domains,
    const std::vector<Constraint>& constraints) {
    std::unordered_map<int, int> assignment;
    if (backtrack(variables, domains, constraints, assignment, 0)) {
        return assignment;
    }
    return {};
}
