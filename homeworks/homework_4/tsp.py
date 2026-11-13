import math
import os
import random
import sys
import time
from itertools import pairwise

POPULATION_SIZE = 300
GENERATIONS = 1000
TOURNAMENT_SIZE = 3
ELITE_SIZE = 2
MUTATION_RATE = 0.5
CHECKPOINTS = 10
PLANE_SIZE = 1000


def read_cities():
    tokens = sys.stdin.read().split()
    if tokens[0].isdigit():
        count = int(tokens[0])
        return None, [(random.uniform(0, PLANE_SIZE), random.uniform(0, PLANE_SIZE)) for _ in range(count)]

    count = int(tokens[1])
    rows = [tokens[2 + 3 * city:5 + 3 * city] for city in range(count)]
    return [name for name, _, _ in rows], [(float(x), float(y)) for _, x, y in rows]


def order_crossover(first, second):
    start, end = sorted(random.sample(range(len(first) + 1), 2))
    middle = first[start:end]
    taken = set(middle)
    rest = [city for city in second if city not in taken]
    return rest[:start] + middle + rest[start:]


def invert(route):
    start, end = sorted(random.sample(range(len(route) + 1), 2))
    route[start:end] = route[start:end][::-1]


def evolve(points):
    n = len(points)
    distances = [[math.dist(a, b) for b in points] for a in points]

    def individual(route):
        return sum(distances[a][b] for a, b in pairwise(route)), route

    def select(population):
        return min(random.sample(population, TOURNAMENT_SIZE))[1]

    size = min(POPULATION_SIZE, math.factorial(n))
    population = sorted(individual(random.sample(range(n), n)) for _ in range(size))
    history = [population[0][0]]

    for _ in range(GENERATIONS):
        next_generation = population[:ELITE_SIZE]
        seen = {tuple(route) for _, route in next_generation}
        while len(next_generation) < size:
            child = order_crossover(select(population), select(population))
            if random.random() < MUTATION_RATE:
                invert(child)
            if tuple(child) not in seen:
                seen.add(tuple(child))
                next_generation.append(individual(child))

        population = sorted(next_generation)
        history.append(population[0][0])

    return history, population[0][1]


def checkpoints(history):
    return [history[round(step * (len(history) - 1) / (CHECKPOINTS - 1))] for step in range(CHECKPOINTS)]


def main():
    names, points = read_cities()

    start = time.perf_counter()
    history, route = evolve(points)
    print(f"# TIMES_MS: alg={(time.perf_counter() - start) * 1000:.3f}")

    if os.environ.get("FMI_TIME_ONLY") == "1":
        return
    print(*checkpoints(history), sep="\n")
    print()
    if names:
        print(" -> ".join(names[city] for city in route))
    print(history[-1])


if __name__ == "__main__":
    main()
