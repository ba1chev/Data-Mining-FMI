import os
import sys
import time
from itertools import combinations
from math import inf, isqrt

FOUND = -1
MOVES = ((0, 1, "left"), (0, -1, "right"), (1, 0, "up"), (-1, 0, "down"))


def manhattan(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])


def read_puzzle():
    numbers = list(map(int, sys.stdin.read().split()))
    count, zero_index, tiles = numbers[0], numbers[1], numbers[2:]
    goal = list(range(1, count + 1))
    goal.insert(count if zero_index == -1 else zero_index, 0)
    return tiles, goal


def is_solvable(tiles, goal):
    size = isqrt(len(tiles))
    position = {tile: index for index, tile in enumerate(goal)}
    permutation = [position[tile] for tile in tiles]
    inversions = sum(a > b for a, b in combinations(permutation, 2))
    blank_distance = manhattan(divmod(tiles.index(0), size), divmod(goal.index(0), size))
    return (inversions + blank_distance) % 2 == 0


def ida_star(tiles, goal):
    size = isqrt(len(tiles))
    cells = range(len(tiles))
    target = {tile: divmod(index, size) for index, tile in enumerate(goal)}
    distance = [[manhattan(divmod(cell, size), target[tile]) if tile else 0 for cell in cells]
                for tile in range(len(tiles))]

    neighbours = []
    for cell in cells:
        row, col = divmod(cell, size)
        neighbours.append([((row + dr) * size + col + dc, move)
                           for dr, dc, move in MOVES
                           if 0 <= row + dr < size and 0 <= col + dc < size])

    state = list(tiles)
    path = []

    def search(blank, previous, g, h, bound):
        if g + h > bound:
            return g + h
        if h == 0:
            return FOUND

        least = inf
        for cell, move in neighbours[blank]:
            if cell == previous:
                continue
            tile = state[cell]
            state[blank], state[cell] = tile, 0
            path.append(move)

            result = search(cell, blank, g + 1, h - distance[tile][cell] + distance[tile][blank], bound)
            if result == FOUND:
                return FOUND

            path.pop()
            state[blank], state[cell] = 0, tile
            least = min(least, result)
        return least

    h = sum(distance[tile][cell] for cell, tile in enumerate(state))
    bound = h
    while bound != FOUND:
        bound = search(state.index(0), None, 0, h, bound)
    return path


def main():
    tiles, goal = read_puzzle()

    start = time.perf_counter()
    path = ida_star(tiles, goal) if is_solvable(tiles, goal) else None
    print(f"# TIMES_MS: alg={(time.perf_counter() - start) * 1000:.3f}")

    if os.environ.get("FMI_TIME_ONLY") == "1":
        return
    if path is None:
        print(-1)
    else:
        print(len(path), *path, sep="\n")


if __name__ == "__main__":
    main()
