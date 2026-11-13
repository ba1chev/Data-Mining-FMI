import os
import random
import sys
import time

CANDIDATES = 32


def min_conflicts(n, max_steps):
    queens = [0] * n
    rows, diagonals, anti_diagonals = [0] * n, [0] * (2 * n), [0] * (2 * n)

    def update(col, delta):
        row = queens[col]
        rows[row] += delta
        diagonals[row - col + n] += delta
        anti_diagonals[row + col] += delta

    def attacks(row, col):
        return rows[row] + diagonals[row - col + n] + anti_diagonals[row + col]

    free_rows = random.sample(range(n), n)
    for col in range(n):
        candidates = range(len(free_rows))[-CANDIDATES:]
        queens[col] = free_rows.pop(min(candidates, key=lambda index: attacks(free_rows[index], col)))
        update(col, 1)

    for _ in range(max_steps):
        conflicted = [col for col, row in enumerate(queens) if attacks(row, col) > 3]
        if not conflicted:
            return queens

        col = random.choice(conflicted)
        update(col, -1)
        scores = [r + d + a for r, d, a in zip(rows, diagonals[n - col:2 * n - col], anti_diagonals[col:col + n])]
        least = min(scores)
        queens[col] = random.choice([row for row, score in enumerate(scores) if score == least])
        update(col, 1)
    return None


def solve(n):
    if n in (2, 3):
        return None
    queens = None
    while queens is None:
        queens = min_conflicts(n, max_steps=2 * n + 100)
    return queens


def board(queens):
    return "\n".join(" ".join("*" if row == queen else "_" for queen in queens) for row in range(len(queens)))


def main():
    n = int(input())

    start = time.perf_counter()
    queens = solve(n)
    print(f"# TIMES_MS: alg={(time.perf_counter() - start) * 1000:.3f}")

    if os.environ.get("FMI_TIME_ONLY") == "1":
        return
    print(-1 if queens is None else queens)
    if queens is not None and "--board" in sys.argv:
        print(board(queens))


if __name__ == "__main__":
    main()
