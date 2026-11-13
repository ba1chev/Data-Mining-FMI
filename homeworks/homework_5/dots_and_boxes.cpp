#include <map>
#include <random>
#include <tuple>
#include <string>
#include <vector>
#include <climits>
#include <cstdint>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

const int SEARCH_DEPTH = 4;
const int EXACT_THRESHOLD = 14;
const int VERTICAL = 1;
const int HORIZONTAL = 2;
const char COMPUTER = 'C';
const char PLAYER = 'P';

struct Edge {
public:
    int kind = 0;
    int row = 0;
    int col = 0;

    Edge(const int kind, const int row, const int col) {
        this->kind = kind;
        this->row = row;
        this->col = col;
    }

    bool operator < (const Edge& other) const {
        return std::tie(kind, row, col) < std::tie(other.kind, other.row, other.col);
    }
};

class Board {
public:
    int rows = 0;
    int cols = 0;
    std::vector<Edge> edges;
    std::map<Edge, int> index;
    std::vector<uint64_t> boxMasks;
    std::vector<std::vector<int>> edgeBoxes;
    std::vector<char> owners;
    uint64_t full = 0;
    uint64_t mask = 0;

    Board(const int rows, const int cols) {
        this->rows = rows;
        this->cols = cols;
        for (int row = 0; row <= rows; row++) {
            for (int col = 0; col < cols; col++) {
                edges.push_back(Edge(HORIZONTAL, row, col));
            }
        }
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col <= cols; col++) {
                edges.push_back(Edge(VERTICAL, row, col));
            }
        }
        for (size_t i = 0; i < edges.size(); i++) {
            index[edges[i]] = i;
        }

        edgeBoxes.assign(edges.size(), {});
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                boxMasks.push_back(bit(Edge(HORIZONTAL, row, col)) | bit(Edge(HORIZONTAL, row + 1, col)) |
                    bit(Edge(VERTICAL, row, col)) | bit(Edge(VERTICAL, row, col + 1)));
            }
        }
        for (size_t box = 0; box < boxMasks.size(); box++) {
            for (size_t edge = 0; edge < edges.size(); edge++) {
                if (boxMasks[box] >> edge & 1) {
                    edgeBoxes[edge].push_back(box);
                }
            }
        }

        owners.assign(boxMasks.size(), ' ');
        full = edges.size() == 64 ? UINT64_MAX : (uint64_t(1) << edges.size()) - 1;
    }

    uint64_t bit(const Edge& edge) const {
        return uint64_t(1) << index.at(edge);
    }

    std::vector<int> freeEdges(const uint64_t currentMask) const {
        std::vector<int> result;
        for (size_t edge = 0; edge < edges.size(); edge++) {
            if (!(currentMask >> edge & 1)) {
                result.push_back(edge);
            }
        }
        return result;
    }

    int remaining(const uint64_t currentMask) const {
        return __builtin_popcountll(full ^ currentMask);
    }

    int sides(const uint64_t currentMask, const int box) const {
        return __builtin_popcountll(currentMask & boxMasks[box]);
    }

    int completed(const uint64_t currentMask, const int edge) const {
        uint64_t nextMask = currentMask | uint64_t(1) << edge;
        int count = 0;
        for (const int currentBox: edgeBoxes[edge]) {
            if ((nextMask & boxMasks[currentBox]) == boxMasks[currentBox]) {
                count++;
            }
        }
        return count;
    }

    bool has(const Edge& edge) const {
        return mask & bit(edge);
    }

    int play(const int edge, const char player) {
        int gained = 0;
        mask |= uint64_t(1) << edge;
        for (const int currentBox: edgeBoxes[edge]) {
            if ((mask & boxMasks[currentBox]) == boxMasks[currentBox]) {
                owners[currentBox] = player;
                gained++;
            }
        }
        return gained;
    }

    bool isOver() const {
        return mask == full;
    }

    int score(const char player) const {
        return std::count(owners.begin(), owners.end(), player);
    }

    void print() const {
        for (int row = 0; row <= rows; row++) {
            std::string line = "o";
            for (int col = 0; col < cols; col++) {
                line += has(Edge(HORIZONTAL, row, col)) ? " - o" : "   o";
            }
            std::cout << line << '\n';

            if (row < rows) {
                line.clear();
                for (int col = 0; col <= cols; col++) {
                    line += has(Edge(VERTICAL, row, col)) ? '|' : ' ';
                    if (col < cols) {
                        line += std::string(" ") + owners[row * cols + col] + " ";
                    }
                }
                std::cout << line.substr(0, line.find_last_not_of(' ') + 1) << '\n';
            }
        }
        std::cout << '\n';
    }
};

class Agent {
private:
    const Board& board;
    std::unordered_map<uint64_t, int> memo;
    std::mt19937 rng;

    std::vector<int> orderedMoves(const uint64_t mask) {
        std::vector<int> moves = board.freeEdges(mask);
        std::shuffle(moves.begin(), moves.end(), rng);

        std::vector<std::pair<std::pair<int, int>, int>> prioritized;
        for (const int currentEdge: moves) {
            uint64_t nextMask = mask | uint64_t(1) << currentEdge;
            int exposed = 0;
            for (const int currentBox: board.edgeBoxes[currentEdge]) {
                exposed += board.sides(nextMask, currentBox) == 3;
            }
            prioritized.push_back({{-board.completed(mask, currentEdge), exposed}, currentEdge});
        }
        std::stable_sort(prioritized.begin(), prioritized.end(),
            [](const auto& first, const auto& second) { return first.first < second.first; });

        for (size_t i = 0; i < moves.size(); i++) {
            moves[i] = prioritized[i].second;
        }
        return moves;
    }

    int evaluate(const uint64_t mask, const int difference, const bool isComputerTurn) const {
        int exposed = 0;
        for (size_t box = 0; box < board.boxMasks.size(); box++) {
            exposed += board.sides(mask, box) == 3;
        }
        return difference + (isComputerTurn ? exposed : -exposed);
    }

    int exactGain(const uint64_t mask) {
        if (mask == board.full) {
            return 0;
        }

        const auto& currentIterator = memo.find(mask);
        if (currentIterator != memo.end()) {
            return currentIterator->second;
        }

        int best = INT_MIN;
        for (const int currentEdge: board.freeEdges(mask)) {
            int gained = board.completed(mask, currentEdge);
            uint64_t nextMask = mask | uint64_t(1) << currentEdge;
            best = std::max(best, gained ? gained + exactGain(nextMask) : -exactGain(nextMask));
        }
        memo[mask] = best;
        return best;
    }

    int minimax(const uint64_t mask, const int difference, const int depth,
        int alpha, int beta, const bool isComputerTurn) {
        if (board.remaining(mask) <= EXACT_THRESHOLD) {
            int gain = exactGain(mask);
            return difference + (isComputerTurn ? gain : -gain);
        }
        if (depth == 0) {
            return evaluate(mask, difference, isComputerTurn);
        }

        int best = isComputerTurn ? INT_MIN : INT_MAX;
        for (const int currentEdge: orderedMoves(mask)) {
            int gained = board.completed(mask, currentEdge);
            bool nextTurn = gained ? isComputerTurn : !isComputerTurn;
            int value = minimax(mask | uint64_t(1) << currentEdge,
                difference + (isComputerTurn ? gained : -gained), depth - 1, alpha, beta, nextTurn);
            if (isComputerTurn) {
                best = std::max(best, value);
                alpha = std::max(alpha, best);
            } else {
                best = std::min(best, value);
                beta = std::min(beta, best);
            }
            if (alpha >= beta) {
                break;
            }
        }
        return best;
    }

public:
    Agent(const Board& board) : board(board), rng(std::random_device{}()) {}

    int bestMove() {
        int bestEdge = -1;
        int alpha = INT_MIN;
        for (const int currentEdge: orderedMoves(board.mask)) {
            int gained = board.completed(board.mask, currentEdge);
            int value = minimax(board.mask | uint64_t(1) << currentEdge, gained,
                SEARCH_DEPTH - 1, alpha, INT_MAX, gained > 0);
            if (value > alpha) {
                alpha = value;
                bestEdge = currentEdge;
            }
        }
        return bestEdge;
    }
};

std::vector<int> readNumbers(const std::string& prompt, const size_t count) {
    std::string line;
    while (true) {
        std::cout << prompt << std::endl;
        if (!std::getline(std::cin, line)) {
            return {};
        }

        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream stream(line);
        std::vector<int> numbers;
        int number = 0;
        while (stream >> number) {
            numbers.push_back(number);
        }
        if (stream.eof() && numbers.size() == count) {
            return numbers;
        }
    }
}

int readMove(const Board& board) {
    while (true) {
        std::vector<int> numbers = readNumbers("Please enter your move:", 3);
        if (numbers.empty()) {
            return -1;
        }

        Edge edge(numbers[0], numbers[2] - 1, numbers[1] - 1);
        if (board.index.count(edge) && !board.has(edge)) {
            return board.index.at(edge);
        }
        std::cout << "Invalid move!" << std::endl;
    }
}

int main() {
    std::vector<int> size = readNumbers("Enter N, M:", 2);
    std::vector<int> first = readNumbers("Computer(1) or Player(2) is first?", 1);
    if (size.empty() || first.empty()) {
        return 0;
    }
    if (size[0] < 1 || size[1] < 1 || (size[0] + 1) * size[1] + size[0] * (size[1] + 1) > 64) {
        std::cout << "Unsupported board size!" << std::endl;
        return 0;
    }

    Board board(size[0], size[1]);
    Agent agent(board);
    char turn = first[0] == 1 ? COMPUTER : PLAYER;
    board.print();

    while (!board.isOver()) {
        int edge = -1;
        if (turn == PLAYER) {
            edge = readMove(board);
            if (edge == -1) {
                return 0;
            }
        } else {
            edge = agent.bestMove();
            const Edge& chosen = board.edges[edge];
            std::cout << "Computer plays: " << chosen.kind << ' ' << chosen.col + 1 << ' ' << chosen.row + 1 << '\n';
        }

        if (board.play(edge, turn) == 0) {
            turn = turn == PLAYER ? COMPUTER : PLAYER;
        }
        board.print();
    }

    int computer = board.score(COMPUTER);
    int player = board.score(PLAYER);
    std::cout << "Computer: " << computer << ", Player: " << player << '\n';
    std::cout << (computer > player ? "Computer wins!" : player > computer ? "Player wins!" : "Draw!") << '\n';

    return 0;
}
