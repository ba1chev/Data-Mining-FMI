#include <chrono>
#include <cmath>
#include <string>
#include <vector>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <algorithm>

struct Board {
public:
    int size = 0;
    int blank = 0;
    std::vector<int> tiles;
    std::vector<int> goalPositions;
};

typedef int (*HeuristicsFunction)(const Board&);

struct Move {
public:
    int rowShift = 0;
    int colShift = 0;
    std::string name;

    Move(const int rowShift, const int colShift, const std::string& name) {
        this->rowShift = rowShift;
        this->colShift = colShift;
        this->name = name;
    }
};

// The name is the direction the tile moves,
// so the blank moves the opposite way
const std::vector<Move> MOVES = {
    Move(0, 1, "left"), Move(0, -1, "right"), Move(1, 0, "up"), Move(-1, 0, "down")
};

int manhattan(const Board& board) {
    int total = 0;
    for (int cell = 0; cell < board.size * board.size; cell++) {
        int tile = board.tiles[cell];
        if (tile != 0) {
            int goal = board.goalPositions[tile];
            total += std::abs(cell / board.size - goal / board.size) +
                std::abs(cell % board.size - goal % board.size);
        }
    }

    return total;
}

// The parity of the inversions plus the distance of
// the blank to its goal cell must be even, which also
// covers boards with an even side and any goal index
bool isSolvable(const Board& board) {
    int inversions = 0;
    for (size_t i = 0; i < board.tiles.size(); i++) {
        for (size_t j = i + 1; j < board.tiles.size(); j++) {
            if (board.goalPositions[board.tiles[i]] > board.goalPositions[board.tiles[j]]) {
                inversions++;
            }
        }
    }

    int goal = board.goalPositions[0];
    int blankDistance = std::abs(board.blank / board.size - goal / board.size) +
        std::abs(board.blank % board.size - goal % board.size);
    return (inversions + blankDistance) % 2 == 0;
}

bool idaStarRecursive(Board& board, HeuristicsFunction heuristicsFunction,
    const int previousBlank, const int distance, const int tolerance,
    int& newTolerance, std::vector<std::string>& path) {
    int heuristics = heuristicsFunction(board);
    int currentDistance = distance + heuristics;
    if (currentDistance > tolerance) {
        newTolerance = std::min(newTolerance, currentDistance);
        return false;
    }
    if (heuristics == 0) {
        return true;
    }

    int blank = board.blank;
    for (const Move& currentMove: MOVES) {
        int nextRow = blank / board.size + currentMove.rowShift;
        int nextCol = blank % board.size + currentMove.colShift;
        int nextBlank = nextRow * board.size + nextCol;
        // Moving the blank back to where it came
        // from only undoes the last move
        if (nextRow < 0 || nextRow >= board.size || nextCol < 0 ||
            nextCol >= board.size || nextBlank == previousBlank) {
            continue;
        }

        std::swap(board.tiles[blank], board.tiles[nextBlank]);
        board.blank = nextBlank;
        path.push_back(currentMove.name);
        if (idaStarRecursive(board, heuristicsFunction, blank, distance + 1,
            tolerance, newTolerance, path)) {
            return true;
        }

        path.pop_back();
        board.blank = blank;
        std::swap(board.tiles[blank], board.tiles[nextBlank]);
    }

    return false;
}

int idaStarSearch(Board board, HeuristicsFunction heuristicsFunction, std::vector<std::string>& path) {
    int tolerance = heuristicsFunction(board);

    while (true) {
        int nextTolerance = INT_MAX;

        if (idaStarRecursive(board, heuristicsFunction, -1, 0,
            tolerance, nextTolerance, path)) {
            return tolerance;
        }
        if (nextTolerance == INT_MAX) {
            return -1;
        }
        tolerance = nextTolerance;
    }
}

Board readBoard() {
    int N = 0;
    int zeroIndex = 0;
    std::cin >> N >> zeroIndex;

    Board board;
    board.size = std::round(std::sqrt(N + 1));
    board.tiles.resize(N + 1);
    for (int cell = 0; cell <= N; cell++) {
        std::cin >> board.tiles[cell];
        if (board.tiles[cell] == 0) {
            board.blank = cell;
        }
    }

    std::vector<int> goal;
    for (int tile = 1; tile <= N; tile++) {
        goal.push_back(tile);
    }
    goal.insert(goal.begin() + (zeroIndex == -1 ? N : zeroIndex), 0);

    board.goalPositions.resize(N + 1);
    for (int cell = 0; cell <= N; cell++) {
        board.goalPositions[goal[cell]] = cell;
    }

    return board;
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

int main() {
    Board board = readBoard();

    auto start = std::chrono::steady_clock::now();
    std::vector<std::string> path;
    int length = isSolvable(board) ? idaStarSearch(board, manhattan, path) : -1;
    printTime(start);

    if (isTimeOnly()) {
        return 0;
    }

    std::cout << length << '\n';
    for (const std::string& currentMove: path) {
        std::cout << currentMove << '\n';
    }

    return 0;
}
