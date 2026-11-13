#include <random>
#include <climits>
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

int countConflicts(const std::unordered_map<int, int>& assignment,
    const std::vector<Constraint>& constraints, const int variable, const int value) {
    int conflicts = 0;
    for (const Constraint& currentConstraint: constraints) {
        if (currentConstraint.first == variable &&
            !currentConstraint.isSatisfied(value, assignment.at(currentConstraint.second))) {
            conflicts++;
        } else if (currentConstraint.second == variable &&
            !currentConstraint.isSatisfied(assignment.at(currentConstraint.first), value)) {
            conflicts++;
        }
    }
    return conflicts;
}

int pickRandom(const std::vector<int>& candidates, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> distribution(0, candidates.size() - 1);
    return candidates[distribution(rng)];
}

std::unordered_map<int, int> minConflicts(const std::vector<int>& variables,
    std::unordered_map<int, std::vector<int>>& domains,
    const std::vector<Constraint>& constraints, const int maxSteps, std::mt19937& rng) {
    std::unordered_map<int, int> assignment;
    for (const int currentVariable: variables) {
        assignment[currentVariable] = pickRandom(domains[currentVariable], rng);
    }

    for (int step = 0; step < maxSteps; step++) {
        std::vector<int> mostConflicted;
        int maxConflicts = 0;
        for (const int currentVariable: variables) {
            int currentConflicts = countConflicts(assignment, constraints,
                currentVariable, assignment[currentVariable]);
            if (currentConflicts > maxConflicts) {
                maxConflicts = currentConflicts;
                mostConflicted.clear();
            }
            if (currentConflicts > 0 && currentConflicts == maxConflicts) {
                mostConflicted.push_back(currentVariable);
            }
        }
        if (mostConflicted.empty()) {
            return assignment;
        }

        const int currentVariable = pickRandom(mostConflicted, rng);
        std::vector<int> bestValues;
        int minConflictsCount = INT_MAX;
        for (const int currentValue: domains[currentVariable]) {
            int currentConflicts = countConflicts(assignment, constraints, currentVariable, currentValue);
            if (currentConflicts < minConflictsCount) {
                minConflictsCount = currentConflicts;
                bestValues.clear();
            }
            if (currentConflicts == minConflictsCount) {
                bestValues.push_back(currentValue);
            }
        }
        assignment[currentVariable] = pickRandom(bestValues, rng);
    }

    return {};
}
