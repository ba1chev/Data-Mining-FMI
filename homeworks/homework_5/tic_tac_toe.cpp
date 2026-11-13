#include <chrono>
#include <limits>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <algorithm>

using Move = std::pair<int, int>;
using Board = std::vector<std::vector<std::string>>;

const int SIZE = 3;
const int WIN_SCORE = 10;
const int INF = std::numeric_limits<int>::max();
const std::string EMPTY = "_";
const Move NO_MOVE = {-1, -1};

const std::vector<Move> MOVE_ORDER {
    {1, 1}, {0, 0}, {0, 2}, {2, 0}, {2, 2}, {0, 1}, {1, 0}, {1, 2}, {2, 1}
};

const std::vector<std::vector<Move>> LINES {
    {{0, 0}, {0, 1}, {0, 2}}, {{1, 0}, {1, 1}, {1, 2}}, {{2, 0}, {2, 1}, {2, 2}},
    {{0, 0}, {1, 0}, {2, 0}}, {{0, 1}, {1, 1}, {2, 1}}, {{0, 2}, {1, 2}, {2, 2}},
    {{0, 0}, {1, 1}, {2, 2}}, {{0, 2}, {1, 1}, {2, 0}}
};

std::string opponent(const std::string& player) {
    return player == "X" ? "O" : "X";
}

struct Node {
    std::vector<Node*> children;
    std::vector<std::vector<std::string>> state;
    std::string turn;
    Move move;

    Node(const Board& state, const std::string& turn, const Move& move = NO_MOVE)
        : state(state), turn(turn), move(move) {}

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    ~Node() {
        for (Node* child : children) {
            delete child;
        }
    }

    const std::string& at(const Move& cell) const {
        return state[cell.first][cell.second];
    }

    std::string winner() const {
        for (const std::vector<Move>& line : LINES) {
            const std::string& first = at(line[0]);
            if (first != EMPTY && first == at(line[1]) && first == at(line[2])) {
                return first;
            }
        }
        return EMPTY;
    }

    bool isFull() const {
        for (const std::vector<std::string>& row : state) {
            if (std::find(row.begin(), row.end(), EMPTY) != row.end()) {
                return false;
            }
        }
        return true;
    }

    bool isTerminal() const {
        return winner() != EMPTY || isFull();
    }

    std::vector<Node*>& expand() {
        if (children.empty() && !isTerminal()) {
            for (const Move& cell : MOVE_ORDER) {
                if (at(cell) == EMPTY) {
                    Board next = state;
                    next[cell.first][cell.second] = turn;
                    children.push_back(new Node(next, opponent(turn), cell));
                }
            }
        }
        return children;
    }
};

class GameTree {
private:
    Node* root = nullptr;

    int evaluate(const Node* node, int depth, const std::string& maximizer) const {
        const std::string winner = node->winner();
        if (winner == EMPTY) {
            return 0;
        }
        return winner == maximizer ? WIN_SCORE - depth : depth - WIN_SCORE;
    }

    int minimax(Node* node, int depth, int alpha, int beta, const std::string& maximizer) {
        if (node->isTerminal()) {
            return evaluate(node, depth, maximizer);
        }

        const bool maximizing = node->turn == maximizer;
        int best = maximizing ? -INF : INF;
        for (Node* child : node->expand()) {
            const int score = minimax(child, depth + 1, alpha, beta, maximizer);
            if (maximizing) {
                best = std::max(best, score);
                alpha = std::max(alpha, best);
            } else {
                best = std::min(best, score);
                beta = std::min(beta, best);
            }
            if (alpha >= beta) {
                break;
            }
        }
        return best;
    }

public:
    GameTree(const Board& state, const std::string& turn) : root(new Node(state, turn)) {}

    GameTree(const GameTree&) = delete;
    GameTree& operator=(const GameTree&) = delete;

    ~GameTree() {
        delete root;
    }

    const Node& current() const {
        return *root;
    }

    Move bestMove() {
        Move best = NO_MOVE;
        int alpha = -INF;
        for (Node* child : root->expand()) {
            const int score = minimax(child, 1, alpha, INF, root->turn);
            if (score > alpha) {
                alpha = score;
                best = child->move;
            }
        }
        return best;
    }

    bool play(const Move& move) {
        for (Node*& child : root->expand()) {
            if (child->move == move) {
                Node* next = child;
                child = nullptr;
                delete root;
                root = next;
                return true;
            }
        }
        return false;
    }
};

Board readBoard() {
    Board board(SIZE);
    int borders = 0;
    std::string token;
    while (borders <= SIZE && std::cin >> token) {
        if (token.front() == '+') {
            ++borders;
        } else if (token != "|") {
            board[borders - 1].push_back(token);
        }
    }
    return board;
}

void printBoard(const Board& board) {
    const std::string border = "+---+---+---+";
    std::cout << border << '\n';
    for (const std::vector<std::string>& row : board) {
        std::cout << '|';
        for (const std::string& cell : row) {
            std::cout << ' ' << cell << " |";
        }
        std::cout << '\n' << border << '\n';
    }
}

void runJudge() {
    std::string keyword, turn;
    std::cin >> keyword >> turn;

    const auto start = std::chrono::steady_clock::now();
    GameTree tree(readBoard(), turn);
    const bool terminal = tree.current().isTerminal();
    const Move move = terminal ? NO_MOVE : tree.bestMove();
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;

    std::cout << "# TIMES_MS: alg=" << elapsed.count() << '\n';
    if (terminal) {
        std::cout << -1 << '\n';
    } else {
        std::cout << move.first + 1 << ' ' << move.second + 1 << '\n';
    }
}

void runGame() {
    std::string keyword, first, human;
    std::cin >> keyword >> first >> keyword >> human;

    GameTree tree(readBoard(), first);
    while (!tree.current().isTerminal()) {
        Move move = NO_MOVE;
        if (tree.current().turn == human) {
            int row = 0, col = 0;
            if (!(std::cin >> row >> col)) {
                return;
            }
            move = {row - 1, col - 1};
        } else {
            move = tree.bestMove();
        }

        if (!tree.play(move)) {
            std::cout << "INVALID MOVE" << '\n';
            continue;
        }
        printBoard(tree.current().state);
    }

    const std::string winner = tree.current().winner();
    std::cout << (winner == EMPTY ? "DRAW" : "WINNER: " + winner) << '\n';
}

int main() {
    std::string mode;
    std::cin >> mode;

    if (mode == "JUDGE") {
        runJudge();
    } else if (mode == "GAME") {
        runGame();
    }

    return 0;
}
