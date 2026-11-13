import sys
import time
from math import inf

EMPTY = "_"
WIN_SCORE = 10
MOVE_ORDER = ((1, 1), (0, 0), (0, 2), (2, 0), (2, 2), (0, 1), (1, 0), (1, 2), (2, 1))
LINES = (
    ((0, 0), (0, 1), (0, 2)), ((1, 0), (1, 1), (1, 2)), ((2, 0), (2, 1), (2, 2)),
    ((0, 0), (1, 0), (2, 0)), ((0, 1), (1, 1), (2, 1)), ((0, 2), (1, 2), (2, 2)),
    ((0, 0), (1, 1), (2, 2)), ((0, 2), (1, 1), (2, 0)),
)


def opponent(player):
    return "O" if player == "X" else "X"


class Node:
    def __init__(self, state, turn, move=None):
        self.children = []
        self.state = state
        self.turn = turn
        self.move = move

    def at(self, cell):
        row, col = cell
        return self.state[row][col]

    def winner(self):
        for line in LINES:
            first = self.at(line[0])
            if first != EMPTY and all(self.at(cell) == first for cell in line[1:]):
                return first
        return None

    def is_full(self):
        return all(EMPTY not in row for row in self.state)

    def is_terminal(self):
        return self.winner() is not None or self.is_full()

    def expand(self):
        if not self.children and not self.is_terminal():
            for row, col in MOVE_ORDER:
                if self.state[row][col] == EMPTY:
                    state = [list(line) for line in self.state]
                    state[row][col] = self.turn
                    self.children.append(Node(state, opponent(self.turn), (row, col)))
        return self.children


class GameTree:
    def __init__(self, state, turn):
        self.root = Node(state, turn)

    def evaluate(self, node, depth, maximizer):
        winner = node.winner()
        if winner is None:
            return 0
        return WIN_SCORE - depth if winner == maximizer else depth - WIN_SCORE

    def minimax(self, node, depth, alpha, beta, maximizer):
        if node.is_terminal():
            return self.evaluate(node, depth, maximizer)

        maximizing = node.turn == maximizer
        best = -inf if maximizing else inf
        for child in node.expand():
            score = self.minimax(child, depth + 1, alpha, beta, maximizer)
            if maximizing:
                best = max(best, score)
                alpha = max(alpha, best)
            else:
                best = min(best, score)
                beta = min(beta, best)
            if alpha >= beta:
                break
        return best

    def best_move(self):
        best, alpha = None, -inf
        for child in self.root.expand():
            score = self.minimax(child, 1, alpha, inf, self.root.turn)
            if score > alpha:
                best, alpha = child.move, score
        return best

    def play(self, move):
        for child in self.root.expand():
            if child.move == move:
                self.root = child
                return True
        return False


def read_board(tokens):
    cells, borders = [], 0
    while borders < 4:
        token = next(tokens)
        if token.startswith("+"):
            borders += 1
        elif token != "|":
            cells.append(token)
    return [cells[row:row + 3] for row in range(0, 9, 3)]


def frame(board):
    border = "+---+---+---+"
    return "\n".join([border] + [f"| {' | '.join(row)} |\n{border}" for row in board])


def run_judge(tokens):
    _, turn = next(tokens), next(tokens)

    start = time.perf_counter()
    tree = GameTree(read_board(tokens), turn)
    move = None if tree.root.is_terminal() else tree.best_move()
    print(f"# TIMES_MS: alg={(time.perf_counter() - start) * 1000:.3f}")

    print(-1 if move is None else f"{move[0] + 1} {move[1] + 1}")


def run_game(tokens):
    _, first, _, human = next(tokens), next(tokens), next(tokens), next(tokens)
    tree = GameTree(read_board(tokens), first)

    while not tree.root.is_terminal():
        if tree.root.turn == human:
            move = (int(next(tokens)) - 1, int(next(tokens)) - 1)
        else:
            move = tree.best_move()

        if tree.play(move):
            print(frame(tree.root.state), flush=True)
        else:
            print("INVALID MOVE", flush=True)

    winner = tree.root.winner()
    print(f"WINNER: {winner}" if winner else "DRAW")


def read_tokens():
    for line in sys.stdin:
        yield from line.split()


def main():
    tokens = read_tokens()
    mode = next(tokens)
    if mode == "JUDGE":
        run_judge(tokens)
    elif mode == "GAME":
        try:
            run_game(tokens)
        except (StopIteration, ValueError):
            pass


if __name__ == "__main__":
    main()
