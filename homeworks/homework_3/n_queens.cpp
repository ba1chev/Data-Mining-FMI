#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

const size_t CANDIDATES = 32;

struct Board {
public:
    int n = 0;
    std::vector<int> queens;
    std::vector<int> rows;
    std::vector<int> diagonals;
    std::vector<int> antiDiagonals;

    Board(const int n) {
        this->n = n;
        this->queens.assign(n, 0);
        this->rows.assign(n, 0);
        this->diagonals.assign(2 * n, 0);
        this->antiDiagonals.assign(2 * n, 0);
    }

    int attacks(const int row, const int col) const {
        return rows[row] + diagonals[row - col + n] + antiDiagonals[row + col];
    }

    void update(const int col, const int delta) {
        int row = queens[col];
        rows[row] += delta;
        diagonals[row - col + n] += delta;
        antiDiagonals[row + col] += delta;
    }

    void place(const int col, const int row) {
        queens[col] = row;
        update(col, 1);
    }
};

int pickRandom(const std::vector<int>& candidates, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> distribution(0, candidates.size() - 1);
    return candidates[distribution(rng)];
}

void initialize(Board& board, std::mt19937& rng) {
    std::vector<int> freeRows(board.n);
    for (int row = 0; row < board.n; row++) {
        freeRows[row] = row;
    }
    std::shuffle(freeRows.begin(), freeRows.end(), rng);

    for (int col = 0; col < board.n; col++) {
        size_t bestIndex = freeRows.size() - 1;
        size_t firstCandidate = freeRows.size() > CANDIDATES ? freeRows.size() - CANDIDATES : 0;
        for (size_t i = freeRows.size() - 1; i + 1 > firstCandidate; i--) {
            if (board.attacks(freeRows[i], col) < board.attacks(freeRows[bestIndex], col)) {
                bestIndex = i;
            }
        }

        board.place(col, freeRows[bestIndex]);
        freeRows[bestIndex] = freeRows.back();
        freeRows.pop_back();
    }
}

bool minConflicts(Board& board, const int maxSteps, std::mt19937& rng) {
    initialize(board, rng);
    for (int step = 0; step < maxSteps; step++) {
        std::vector<int> conflicted;
        for (int col = 0; col < board.n; col++) {
            if (board.attacks(board.queens[col], col) > 3) {
                conflicted.push_back(col);
            }
        }
        if (conflicted.empty()) {
            return true;
        }

        int col = pickRandom(conflicted, rng);
        board.update(col, -1);

        std::vector<int> bestRows;
        int minAttacks = INT_MAX;
        for (int row = 0; row < board.n; row++) {
            int currentAttacks = board.attacks(row, col);
            if (currentAttacks < minAttacks) {
                minAttacks = currentAttacks;
                bestRows.clear();
            }
            if (currentAttacks == minAttacks) {
                bestRows.push_back(row);
            }
        }

        board.place(col, pickRandom(bestRows, rng));
    }
    return false;
}

std::vector<int> solve(const int n) {
    if (n == 2 || n == 3) {
        return {};
    }

    std::mt19937 rng(std::random_device{}());
    while (true) {
        Board board(n);
        if (minConflicts(board, 2 * n + 100, rng)) {
            return board.queens;
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

int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n = 0;
    std::cin >> n;

    auto start = std::chrono::steady_clock::now();
    std::vector<int> queens = solve(n);
    std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "# TIMES_MS: alg=" << elapsed.count() << '\n';

    const char* timeOnly = std::getenv("FMI_TIME_ONLY");
    if (timeOnly != nullptr && std::string(timeOnly) == "1") {
        return 0;
    }

    if (queens.empty()) {
        std::cout << -1 << '\n';
        return 0;
    }

    std::cout << '[';
    for (size_t col = 0; col < queens.size(); col++) {
        std::cout << (col > 0 ? ", " : "") << queens[col];
    }
    std::cout << "]\n";

    if (argc > 1 && std::strcmp(argv[1], "--board") == 0) {
        printBoard(queens);
    }

    return 0;
}
