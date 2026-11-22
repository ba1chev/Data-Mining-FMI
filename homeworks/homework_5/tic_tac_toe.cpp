#include <chrono>
#include <string>
#include <vector>
#include <climits>
#include <iostream>
#include <algorithm>

// Parameters of the game:
// 1) side of the board
// 2) score of a win, decreased by its depth
// 3) symbol of an empty cell
namespace Parameters {
    const size_t SIZE = 3;
    const int WIN_SCORE = 10;
    const char EMPTY = '_';
}

// Defining an abstraction
// for the moves
struct Move {
public:
    size_t row = 0;
    size_t col = 0;
};

// Defining an abstraction for the board,
// the player on turn is part of the state
struct Board {
public:
    std::vector<char> cells = std::vector<char>(Parameters::SIZE * Parameters::SIZE, Parameters::EMPTY);
    char turn = 'X';

    char& at(const Move& move) {
        return cells[move.row * Parameters::SIZE + move.col];
    }

    char at(const Move& move) const {
        return cells[move.row * Parameters::SIZE + move.col];
    }
};

// The center first, then the corners and then the edges:
// 1) the stronger moves are tried first, so more branches are pruned
// 2) among equally good moves the first one is played, so the agent is deterministic
const std::vector<Move> MOVE_ORDER {
    {1, 1}, {0, 0}, {0, 2}, {2, 0}, {2, 2}, {0, 1}, {1, 0}, {1, 2}, {2, 1}
};

const std::vector<std::vector<Move>> LINES {
    {{0, 0}, {0, 1}, {0, 2}}, {{1, 0}, {1, 1}, {1, 2}}, {{2, 0}, {2, 1}, {2, 2}},
    {{0, 0}, {1, 0}, {2, 0}}, {{0, 1}, {1, 1}, {2, 1}}, {{0, 2}, {1, 2}, {2, 2}},
    {{0, 0}, {1, 1}, {2, 2}}, {{0, 2}, {1, 1}, {2, 0}}
};

// Defining an abstraction for the
// interval of the alpha-beta pruning,
// every node gets its own copy, so
// narrowing it never leaks to the siblings
struct AlphaBetaInterval {
public:
    int alpha = INT_MIN;
    int beta = INT_MAX;

    void raise(const int value) {
        alpha = std::max(alpha, value);
    }

    void lower(const int value) {
        beta = std::min(beta, value);
    }

    // The other player already has a better
    // choice, so this node is never reached
    bool isEmpty() const {
        return alpha >= beta;
    }
};

// Defining an abstraction for the search tree:
// 1) the tree keeps one state and the path of moves to the current node
// 2) forward and backward walk the tree without copying the state
// 3) the rules of the game are left to the derived trees
template <class S, class M>
class SearchTree {
protected:
    S state;
    std::vector<M> path;

    virtual void apply(const M& move) = 0;
    virtual void undo(const M& move) = 0;

public:
    SearchTree(const S& state) {
        this->state = state;
    }

    virtual ~SearchTree() = default;

    const S& getState() const {
        return state;
    }

    // Playing a move moves the root itself,
    // so the path stays empty between searches
    void play(const M& move) {
        apply(move);
    }

    void forward(const M& move) {
        apply(move);
        path.push_back(move);
    }

    void backward() {
        undo(path.back());
        path.pop_back();
    }

    size_t depth() const {
        return path.size();
    }

    virtual std::vector<M> moves() const = 0;
    virtual bool isTerminal() const = 0;
    virtual char player() const = 0;
    virtual int utility(const char player) const = 0;
    virtual M bestMove() = 0;
};

template <class S, class M>
class MinMaxSearchTree: public SearchTree<S, M> {
private:
    // A win on a smaller depth scores more
    // and a loss on a smaller depth scores less,
    // so the agent wins fast and loses slow
    int score(const char maximizer) const {
        int value = this->utility(maximizer);
        int depth = static_cast<int>(this->depth());
        if (value > 0) {
            return Parameters::WIN_SCORE - depth;
        }
        if (value < 0) {
            return depth - Parameters::WIN_SCORE;
        }

        return 0;
    }

    int minimax(AlphaBetaInterval interval, const char maximizer) {
        if (this->isTerminal()) {
            return score(maximizer);
        }

        bool maximizing = this->player() == maximizer;
        int best = maximizing ? INT_MIN : INT_MAX;
        for (const M& move: this->moves()) {
            this->forward(move);
            int value = minimax(interval, maximizer);
            this->backward();

            if (maximizing) {
                best = std::max(best, value);
                interval.raise(best);
            } else {
                best = std::min(best, value);
                interval.lower(best);
            }
            if (interval.isEmpty()) {
                break;
            }
        }

        return best;
    }

public:
    MinMaxSearchTree(const S& state): 
        SearchTree<S, M>(state) {}

    // The interval of the root is passed down,
    // so a move is replaced only by a strictly
    // better one and ties keep the first move
    M bestMove() override {
        char maximizer = this->player();
        AlphaBetaInterval interval;
        M best = M{};
        for (const M& move: this->moves()) {
            this->forward(move);
            int value = minimax(interval, maximizer);
            this->backward();

            if (value > interval.alpha) {
                interval.raise(value);
                best = move;
            }
        }

        return best;
    }
};

char opponent(const char player) {
    return player == 'X' ? 'O' : 'X';
}

class TicTacToeSearchTree: public MinMaxSearchTree<Board, Move> {
protected:
    void apply(const Move& move) override {
        state.at(move) = state.turn;
        state.turn = opponent(state.turn);
    }

    void undo(const Move& move) override {
        state.at(move) = Parameters::EMPTY;
        state.turn = opponent(state.turn);
    }

public:
    TicTacToeSearchTree(const Board& board): 
        MinMaxSearchTree<Board, Move>(board) {}

    char winner() const {
        for (const std::vector<Move>& line: LINES) {
            char first = state.at(line[0]);
            if (first != Parameters::EMPTY && first == state.at(line[1]) && first == state.at(line[2])) {
                return first;
            }
        }

        return Parameters::EMPTY;
    }

    bool isValid(const int row, const int col) const {
        return row >= 0 && row < static_cast<int>(Parameters::SIZE) &&
            col >= 0 && col < static_cast<int>(Parameters::SIZE) &&
            state.at({static_cast<size_t>(row), static_cast<size_t>(col)}) == Parameters::EMPTY;
    }

    std::vector<Move> moves() const override {
        std::vector<Move> result;
        for (const Move& move: MOVE_ORDER) {
            if (state.at(move) == Parameters::EMPTY) {
                result.push_back(move);
            }
        }

        return result;
    }

    bool isTerminal() const override {
        return winner() != Parameters::EMPTY || moves().empty();
    }

    char player() const override {
        return state.turn;
    }

    int utility(const char player) const override {
        char boardWinner = winner();
        if (boardWinner == Parameters::EMPTY) {
            return 0;
        }

        return boardWinner == player ? 1 : -1;
    }
};

// The board is read until its last border,
// so the next input starts right after it
Board readBoard(const char turn) {
    Board board;
    board.turn = turn;

    size_t cell = 0;
    size_t borders = 0;
    std::string token;
    while (borders <= Parameters::SIZE && std::cin >> token) {
        if (token.front() == '+') {
            borders++;
        } else if (token != "|" && cell < board.cells.size()) {
            board.cells[cell++] = token.front();
        }
    }

    return board;
}

void printBoard(const Board& board) {
    const std::string border = "+---+---+---+";
    std::cout << border << '\n';
    for (size_t row = 0; row < Parameters::SIZE; row++) {
        std::cout << '|';
        for (size_t col = 0; col < Parameters::SIZE; col++) {
            std::cout << ' ' << board.at({row, col}) << " |";
        }
        std::cout << '\n' << border << '\n';
    }
}

void printTime(const std::chrono::steady_clock::time_point& start) {
    std::chrono::duration<double, std::milli> elapsed =
        std::chrono::steady_clock::now() - start;
    std::cout << "# TIMES_MS: alg=" << elapsed.count() << '\n';
}

void runJudge() {
    std::string keyword;
    char turn = 'X';
    std::cin >> keyword >> turn;
    TicTacToeSearchTree tree(readBoard(turn));

    auto start = std::chrono::steady_clock::now();
    bool terminal = tree.isTerminal();
    Move move = terminal ? Move{} : tree.bestMove();
    printTime(start);

    if (terminal) {
        std::cout << -1 << '\n';
        return;
    }
    std::cout << move.row + 1 << ' ' << move.col + 1 << '\n';
}

void runGame() {
    std::string keyword;
    char first = 'X';
    char human = 'O';
    std::cin >> keyword >> first >> keyword >> human;
    TicTacToeSearchTree tree(readBoard(first));

    while (!tree.isTerminal()) {
        Move move;
        if (tree.player() == human) {
            int row = 0;
            int col = 0;
            if (!(std::cin >> row >> col)) {
                return;
            }
            if (!tree.isValid(row - 1, col - 1)) {
                std::cout << "INVALID MOVE" << '\n';
                continue;
            }
            move = {static_cast<size_t>(row - 1), static_cast<size_t>(col - 1)};
        } else {
            move = tree.bestMove();
        }

        tree.play(move);
        printBoard(tree.getState());
    }

    char winner = tree.winner();
    if (winner == Parameters::EMPTY) {
        std::cout << "DRAW" << '\n';
    } else {
        std::cout << "WINNER: " << winner << '\n';
    }
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
