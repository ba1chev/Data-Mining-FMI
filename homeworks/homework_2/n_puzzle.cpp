#include <cmath>
#include <chrono>
#include <string>
#include <vector>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <algorithm>

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

const std::vector<Move> MOVES = {
    Move(0, 1, "left"), Move(0, -1, "right"), Move(1, 0, "up"), Move(-1, 0, "down")
};

int manhattan(const int firstCell, const int secondCell, const int size) {
    return std::abs(firstCell / size - secondCell / size) + std::abs(firstCell % size - secondCell % size);
}

int totalManhattan(const std::vector<int>& tiles, const std::vector<int>& goalPositions, const int size) {
    int total = 0;
    for (size_t cell = 0; cell < tiles.size(); cell++) {
        if (tiles[cell] != 0) {
            total += manhattan(cell, goalPositions[tiles[cell]], size);
        }
    }
    return total;
}

bool isSolvable(const std::vector<int>& tiles, const std::vector<int>& goalPositions, const int size) {
    int inversions = 0;
    for (size_t i = 0; i < tiles.size(); i++) {
        for (size_t j = i + 1; j < tiles.size(); j++) {
            if (goalPositions[tiles[i]] > goalPositions[tiles[j]]) {
                inversions++;
            }
        }
    }

    int blankCell = std::find(tiles.begin(), tiles.end(), 0) - tiles.begin();
    return (inversions + manhattan(blankCell, goalPositions[0], size)) % 2 == 0;
}

bool idaStarRecursive(std::vector<int>& tiles, const std::vector<int>& goalPositions, const int size,
    const int blankCell, const int previousCell, const int distance, const int heuristics,
    const int tolerance, int& newTolerance, std::vector<std::string>& path) {
    int currentDistance = distance + heuristics;
    if (currentDistance > tolerance) {
        newTolerance = std::min(newTolerance, currentDistance);
        return false;
    }
    if (heuristics == 0) {
        return true;
    }

    int blankRow = blankCell / size;
    int blankCol = blankCell % size;
    for (const Move& currentMove: MOVES) {
        int nextRow = blankRow + currentMove.rowShift;
        int nextCol = blankCol + currentMove.colShift;
        int nextCell = nextRow * size + nextCol;
        if (nextRow < 0 || nextRow >= size || nextCol < 0 || nextCol >= size || nextCell == previousCell) {
            continue;
        }

        int tile = tiles[nextCell];
        int nextHeuristics = heuristics - manhattan(nextCell, goalPositions[tile], size)
            + manhattan(blankCell, goalPositions[tile], size);
        std::swap(tiles[blankCell], tiles[nextCell]);
        path.push_back(currentMove.name);

        if (idaStarRecursive(tiles, goalPositions, size, nextCell, blankCell, distance + 1,
            nextHeuristics, tolerance, newTolerance, path)) {
            return true;
        }

        path.pop_back();
        std::swap(tiles[blankCell], tiles[nextCell]);
    }

    return false;
}

std::vector<std::string> idaStarSearch(std::vector<int> tiles, const std::vector<int>& goalPositions, const int size) {
    std::vector<std::string> path;
    int blankCell = std::find(tiles.begin(), tiles.end(), 0) - tiles.begin();
    int heuristics = totalManhattan(tiles, goalPositions, size);
    int tolerance = heuristics;

    while (true) {
        int newTolerance = INT_MAX;
        if (idaStarRecursive(tiles, goalPositions, size, blankCell, -1, 0,
            heuristics, tolerance, newTolerance, path)) {
            return path;
        }
        tolerance = newTolerance;
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int N = 0;
    int zeroIndex = 0;
    std::cin >> N >> zeroIndex;
    std::vector<int> tiles(N + 1);
    for (int& currentTile: tiles) {
        std::cin >> currentTile;
    }

    int size = std::round(std::sqrt(N + 1));
    std::vector<int> goal;
    for (int tile = 1; tile <= N; tile++) {
        goal.push_back(tile);
    }
    goal.insert(goal.begin() + (zeroIndex == -1 ? N : zeroIndex), 0);

    std::vector<int> goalPositions(N + 1);
    for (int cell = 0; cell <= N; cell++) {
        goalPositions[goal[cell]] = cell;
    }

    auto start = std::chrono::steady_clock::now();
    bool solvable = isSolvable(tiles, goalPositions, size);
    std::vector<std::string> path;
    if (solvable) {
        path = idaStarSearch(tiles, goalPositions, size);
    }
    std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "# TIMES_MS: alg=" << elapsed.count() << '\n';

    const char* timeOnly = std::getenv("FMI_TIME_ONLY");
    if (timeOnly != nullptr && std::string(timeOnly) == "1") {
        return 0;
    }

    if (!solvable) {
        std::cout << -1 << '\n';
        return 0;
    }
    std::cout << path.size() << '\n';
    for (const std::string& currentMove: path) {
        std::cout << currentMove << '\n';
    }

    return 0;
}
