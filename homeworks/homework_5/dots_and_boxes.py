import random
from math import inf

SEARCH_DEPTH = 4
EXACT_THRESHOLD = 14
VERTICAL, HORIZONTAL = 1, 2
COMPUTER, PLAYER = "C", "P"


class Board:
    def __init__(self, rows, cols):
        self.rows, self.cols = rows, cols
        self.edges = ([(HORIZONTAL, row, col) for row in range(rows + 1) for col in range(cols)]
                      + [(VERTICAL, row, col) for row in range(rows) for col in range(cols + 1)])
        self.index = {edge: position for position, edge in enumerate(self.edges)}
        self.boxes = [(row, col) for row in range(rows) for col in range(cols)]
        self.box_masks = [self.bits((HORIZONTAL, row, col), (HORIZONTAL, row + 1, col),
                                    (VERTICAL, row, col), (VERTICAL, row, col + 1))
                          for row, col in self.boxes]
        self.edge_boxes = [[box for box, box_mask in enumerate(self.box_masks) if box_mask >> edge & 1]
                           for edge in range(len(self.edges))]
        self.full = (1 << len(self.edges)) - 1
        self.mask = 0
        self.owners = {}

    def bits(self, *edges):
        return sum(1 << self.index[edge] for edge in edges)

    def free_edges(self, mask):
        return [edge for edge in range(len(self.edges)) if not mask >> edge & 1]

    def remaining(self, mask):
        return (self.full ^ mask).bit_count()

    def sides(self, mask, box):
        return (mask & self.box_masks[box]).bit_count()

    def completed(self, mask, edge):
        mask |= 1 << edge
        return [box for box in self.edge_boxes[edge] if mask & self.box_masks[box] == self.box_masks[box]]

    def has(self, edge):
        return self.mask >> self.index[edge] & 1

    def play(self, edge, player):
        boxes = self.completed(self.mask, edge)
        self.mask |= 1 << edge
        for box in boxes:
            self.owners[self.boxes[box]] = player
        return len(boxes)

    def is_over(self):
        return self.mask == self.full

    def score(self, player):
        return sum(owner == player for owner in self.owners.values())

    def render(self):
        lines = []
        for row in range(self.rows + 1):
            lines.append("o" + "".join((" - " if self.has((HORIZONTAL, row, col)) else "   ") + "o"
                                       for col in range(self.cols)))
            if row < self.rows:
                walls = ["|" if self.has((VERTICAL, row, col)) else " " for col in range(self.cols + 1)]
                owners = [f" {self.owners.get((row, col), ' ')} " for col in range(self.cols)] + [""]
                lines.append("".join(wall + owner for wall, owner in zip(walls, owners)).rstrip())
        return "\n".join(lines) + "\n"


class Agent:
    def __init__(self, board):
        self.board = board
        self.memo = {}

    def ordered_moves(self, mask):
        board = self.board

        def priority(edge):
            after = mask | 1 << edge
            captured = len(board.completed(mask, edge))
            exposed = sum(board.sides(after, box) == 3 for box in board.edge_boxes[edge])
            return -captured, exposed

        moves = board.free_edges(mask)
        random.shuffle(moves)
        return sorted(moves, key=priority)

    def evaluate(self, mask, diff, computer_turn):
        exposed = sum(self.board.sides(mask, box) == 3 for box in range(len(self.board.boxes)))
        return diff + (exposed if computer_turn else -exposed)

    def exact_gain(self, mask):
        if mask == self.board.full:
            return 0
        if mask not in self.memo:
            best = -inf
            for edge in self.board.free_edges(mask):
                gained = len(self.board.completed(mask, edge))
                child = mask | 1 << edge
                best = max(best, gained + self.exact_gain(child) if gained else -self.exact_gain(child))
            self.memo[mask] = best
        return self.memo[mask]

    def minimax(self, mask, diff, depth, alpha, beta, computer_turn):
        if self.board.remaining(mask) <= EXACT_THRESHOLD:
            gain = self.exact_gain(mask)
            return diff + (gain if computer_turn else -gain)
        if depth == 0:
            return self.evaluate(mask, diff, computer_turn)

        best = -inf if computer_turn else inf
        for edge in self.ordered_moves(mask):
            gained = len(self.board.completed(mask, edge))
            next_turn = computer_turn if gained else not computer_turn
            value = self.minimax(mask | 1 << edge, diff + (gained if computer_turn else -gained),
                                 depth - 1, alpha, beta, next_turn)
            if computer_turn:
                best = max(best, value)
                alpha = max(alpha, best)
            else:
                best = min(best, value)
                beta = min(beta, best)
            if alpha >= beta:
                break
        return best

    def best_move(self):
        mask = self.board.mask
        best_edge, alpha = None, -inf
        for edge in self.ordered_moves(mask):
            gained = len(self.board.completed(mask, edge))
            value = self.minimax(mask | 1 << edge, gained, SEARCH_DEPTH - 1, alpha, inf, bool(gained))
            if value > alpha:
                best_edge, alpha = edge, value
        return best_edge


def read_numbers(prompt, count):
    while True:
        print(prompt)
        try:
            numbers = list(map(int, input().replace(",", " ").split()))
        except ValueError:
            continue
        if len(numbers) == count:
            return numbers


def read_move(board):
    while True:
        kind, col, row = read_numbers("Please enter your move:", 3)
        edge = (kind, row - 1, col - 1)
        if edge in board.index and not board.has(edge):
            return board.index[edge]
        print("Invalid move!")


def describe(board, edge):
    kind, row, col = board.edges[edge]
    return f"{kind} {col + 1} {row + 1}"


def play(board, agent, turn):
    print(board.render())
    while not board.is_over():
        if turn == PLAYER:
            edge = read_move(board)
        else:
            edge = agent.best_move()
            print(f"Computer plays: {describe(board, edge)}")

        if not board.play(edge, turn):
            turn = COMPUTER if turn == PLAYER else PLAYER
        print(board.render())

    computer, player = board.score(COMPUTER), board.score(PLAYER)
    print(f"Computer: {computer}, Player: {player}")
    print("Computer wins!" if computer > player else "Player wins!" if player > computer else "Draw!")


def main():
    try:
        rows, cols = read_numbers("Enter N, M:", 2)
        first, = read_numbers("Computer(1) or Player(2) is first?", 1)
        board = Board(rows, cols)
        play(board, Agent(board), COMPUTER if first == 1 else PLAYER)
    except EOFError:
        pass


if __name__ == "__main__":
    main()
