#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <algorithm>

const size_t CANDIDATES = 32;

// Defining an abstaction
// for the variables
template <class T>
struct Variable {
public:
    size_t index = 0;
    T data = T{};
};

// Defining function predicate for 
// the conflicts accross variables
template <class T>
using ConflictPredicateFunction = 
    bool (*)(const Variable<T>&, const Variable<T>&);

// Defining function that maps
// a variable to the line it is on
template <class T>
using LineFunction =
    size_t (*)(const Variable<T>&, const size_t);

// Defining an abstaction for the 
// domain used by variables
template <class T>
struct Domain {
public:
    std::vector<T> data;
};

// Defining an abstraction for
// the constraints
template <class T>
class Constraint {
private:
    LineFunction<T> line;
    ConflictPredicateFunction<T> predicate;

    size_t N = 0;
    // How many variables are on each line,
    // so conflicts are counted in O(1)
    std::vector<size_t> occupancy;

public:
    Constraint(const ConflictPredicateFunction<T> predicate, const LineFunction<T> line,
        const size_t N, const size_t lines) {
        this->predicate = predicate;
        this->line = line;
        this->N = N;
        this->occupancy.assign(lines, 0);
    }

    void clear() {
        std::fill(occupancy.begin(), occupancy.end(), 0);
    }

    void add(const Variable<T>& variable) {
        occupancy[line(variable, N)]++;
    }

    void remove(const Variable<T>& variable) {
        occupancy[line(variable, N)]--;
    }

    size_t conflicts(const Variable<T>& variable) const {
        return occupancy[line(variable, N)];
    }

    // After sorting by line, the variables
    // sharing a line become neighbours,
    // so checking adjacent pairs is enough
    bool isSatisfiedBy(std::vector<Variable<T>> variables) const {
        std::sort(variables.begin(), variables.end(),
            [this](const Variable<T>& first, const Variable<T>& second) {
                return line(first, N) < line(second, N);
            });
        for (size_t i = 1; i < variables.size(); i++) {
            if (predicate(variables[i - 1], variables[i])) {
                return false;
            }
        }

        return true;
    }
};

template <class V>
const V& pickRandom(const std::vector<V>& values, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> distribution(0, values.size() - 1);
    return values[distribution(rng)];
}

template <class T>
class CSP {
private:
    std::vector<Variable<T>> variables;
    Domain<T> domain;
    std::vector<Constraint<T>> constraints;

    size_t conflicts(const Variable<T>& variable) const {
        size_t result = 0;
        for (const Constraint<T>& constraint: constraints) {
            result += constraint.conflicts(variable);
        }

        return result;
    }

    void place(const size_t index, const T& value) {
        variables[index].data = value;
        for (Constraint<T>& constraint: constraints) {
            constraint.add(variables[index]);
        }
    }

    void displace(const size_t index) {
        for (Constraint<T>& constraint: constraints) {
            constraint.remove(variables[index]);
        }
    }

    void initialize(std::mt19937& rng) {
        for (Constraint<T>& constraint: constraints) {
            constraint.clear();
        }

        // Greedy start:
        // 1) every value is used only once, so no two variables share a row
        // 2) each variable takes the best of the last CANDIDATES unused values
        std::vector<T> values = domain.data;
        std::shuffle(values.begin(), values.end(), rng);
        for (size_t index = 0; index < variables.size(); index++) {
            size_t best = values.size() - 1;
            size_t first = values.size() > CANDIDATES ? values.size() - CANDIDATES : 0;
            for (size_t i = first; i < values.size(); i++) {
                if (conflicts({index, values[i]}) < conflicts({index, values[best]})) {
                    best = i;
                }
            }

            place(index, values[best]);
            values[best] = values.back();
            values.pop_back();
        }
    }

    std::vector<size_t> conflictedVariables() const {
        std::vector<size_t> result;
        for (size_t index = 0; index < variables.size(); index++) {
            // A placed variable counts itself
            // once in every constraint
            if (conflicts(variables[index]) > constraints.size()) {
                result.push_back(index);
            }
        }

        return result;
    }

    T minConflictsValue(const size_t index, std::mt19937& rng) const {
        std::vector<T> bestValues;
        size_t fewestConflicts = SIZE_MAX;
        for (const T& value: domain.data) {
            size_t currentConflicts = conflicts({index, value});
            if (currentConflicts < fewestConflicts) {
                fewestConflicts = currentConflicts;
                bestValues.clear();
            }
            if (currentConflicts == fewestConflicts) {
                bestValues.push_back(value);
            }
        }

        // Ties are broken randomly,
        // otherwise the search can cycle
        return pickRandom(bestValues, rng);
    }

public:
    CSP(const std::vector<Variable<T>>& variables, const Domain<T>& domain,
        const std::vector<Constraint<T>>& constraints) {
        this->domain = domain;
        this->variables = variables;
        this->constraints = constraints;
    }

    bool minConflicts(const size_t maxSteps, std::mt19937& rng) {
        initialize(rng);
        for (size_t step = 0; step < maxSteps; step++) {
            std::vector<size_t> conflicted = conflictedVariables();
            if (conflicted.empty()) {
                return isSolution();
            }

            // The variable is removed first,
            // so it does not count itself
            size_t index = pickRandom(conflicted, rng);
            displace(index);
            place(index, minConflictsValue(index, rng));
        }

        return false;
    }

    bool isSolution() const {
        for (const Constraint<T>& constraint: constraints) {
            if (!constraint.isSatisfiedBy(variables)) {
                return false;
            }
        }

        return true;
    }

    const std::vector<Variable<T>>& getVariables() const {
        return variables;
    }
};

template <class T>
bool sameRow(const Variable<T>& first, const Variable<T>& second) {
    return first.data == second.data;
}

template <class T>
bool sameDiagonal(const Variable<T>& first, const Variable<T>& second) {
    return first.data - static_cast<T>(first.index) == second.data - static_cast<T>(second.index);
}

template <class T>
bool sameAntiDiagonal(const Variable<T>& first, const Variable<T>& second) {
    return first.data + static_cast<T>(first.index) == second.data + static_cast<T>(second.index);
}

template <class T>
size_t rowLine(const Variable<T>& variable, const size_t) {
    return variable.data;
}

// row - col is in [-(N - 1), N - 1],
// shifted by N - 1 to be a valid index
template <class T>
size_t diagonalLine(const Variable<T>& variable, const size_t N) {
    return variable.data + N - 1 - variable.index;
}

// row + col is in [0, 2N - 2]
template <class T>
size_t antiDiagonalLine(const Variable<T>& variable, const size_t) {
    return variable.data + variable.index;
}

template <class T>
std::vector<Variable<T>> generateNxNQueensVariables(const size_t N) {
    std::vector<Variable<T>> result(N);
    for (size_t index = 0; index < N; index++) {
        result[index].index = index;
    }

    return result;
}

template <class T>
Domain<T> generateNxNQueensDomain(const size_t N) {
    Domain<T> result;
    result.data.reserve(N);
    for (size_t row = 0; row < N; row++) {
        result.data.push_back(row);
    }

    return result;
}

// All three constraints:
// 1) at most one queen on each row
// 2) at most one queen on each diagonal
// 3) at most one queen on each anti-diagonal
// Columns need no constraint, every column is a separate variable
template <class T>
std::vector<Constraint<T>> generateNxNQueensConstraints(const size_t N) {
    return {
        Constraint<T>(sameRow<T>, rowLine<T>, N, N),
        Constraint<T>(sameDiagonal<T>, diagonalLine<T>, N, 2 * N),
        Constraint<T>(sameAntiDiagonal<T>, antiDiagonalLine<T>, N, 2 * N)
    };
}

std::vector<int> solve(const size_t N) {
    // No solution exists for N = 2 and N = 3
    if (N == 2 || N == 3) {
        return {};
    }

    std::mt19937 rng(std::random_device{}());
    CSP<int> csp(generateNxNQueensVariables<int>(N), generateNxNQueensDomain<int>(N),
        generateNxNQueensConstraints<int>(N));
    // Stuck in a local minimum, restart
    // from a new greedy start
    while (true) {
        if (csp.minConflicts(2 * N + 100, rng)) {
            std::vector<int> result(N);
            for (const Variable<int>& variable: csp.getVariables()) {
                result[variable.index] = variable.data;
            }
            return result;
        }
    }
}

void printBoard(const std::vector<int>& queens) {
    for (size_t row = 0; row < queens.size(); row++) {
        for (size_t col = 0; col < queens.size(); col++) {
            std::cout << (col > 0 ? " " : "") << (queens[col] == static_cast<int>(row) ? '*' : '_');
        }
        std::cout << '\n';
    }
}

void printSolution(const std::vector<int>& queens) {
    if (queens.empty()) {
        std::cout << -1 << '\n';
        return;
    }

    std::cout << '[';
    for (size_t col = 0; col < queens.size(); col++) {
        std::cout << (col > 0 ? ", " : "") << queens[col];
    }
    std::cout << "]\n";
}

void printTime(const std::chrono::steady_clock::time_point& start) {
    std::chrono::duration<double, std::milli> elapsed = 
        std::chrono::steady_clock::now() - start;
    std::cout << "# TIMES_MS: alg=" << elapsed.count() << '\n';
}

bool isTimeOnly() {
    const char* timeOnly = std::getenv("FMI_TIME_ONLY");
    return timeOnly != nullptr && std::string(timeOnly) == "1";
}

int main(int argc, char* argv[]) {
    size_t N = 0;
    std::cin >> N;

    auto start = std::chrono::steady_clock::now();
    std::vector<int> queens = solve(N);
    printTime(start);

    if (isTimeOnly()) {
        return 0;
    }

    printSolution(queens);
    if (argc > 1 && std::strcmp(argv[1], "--board") == 0) {
        printBoard(queens);
    }

    return 0;
}
