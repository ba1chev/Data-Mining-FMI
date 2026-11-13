import os
import sys
import time

MOVES = ((-1, ">", None), (-2, ">", "<"), (1, "<", None), (2, "<", ">"))


def children(state):
    empty = state.index("_")
    for shift, frog, jumped_over in MOVES:
        source = empty + shift
        if (0 <= source < len(state) and state[source] == frog
                and (jumped_over is None or state[empty + shift // 2] == jumped_over)):
            cells = list(state)
            cells[empty], cells[source] = frog, "_"
            yield "".join(cells)


def is_dead(state):
    left, right = state.split("_")
    return (">><<" in state
            or left.endswith("<<") and ">" in left
            or right.startswith(">>") and "<" in right)


def dfs(state, goal, path):
    path.append(state)
    if state == goal or any(dfs(child, goal, path) for child in children(state) if not is_dead(child)):
        return True
    path.pop()
    return False


def main():
    n = int(input())
    sys.setrecursionlimit(max(1000, 3 * (n + 1) ** 2))

    start = time.perf_counter()
    path = []
    dfs(">" * n + "_" + "<" * n, "<" * n + "_" + ">" * n, path)
    print(f"# TIMES_MS: alg={(time.perf_counter() - start) * 1000:.3f}")

    if os.environ.get("FMI_TIME_ONLY") != "1":
        print(*path, sep="\n")


if __name__ == "__main__":
    main()
