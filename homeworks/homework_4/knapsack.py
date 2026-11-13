import os
import random
import sys
import time
from itertools import compress
from math import inf

POPULATION_SIZE = 200
GENERATIONS = 500
TOURNAMENT_SIZE = 3
ELITE_SIZE = 2
MUTATION_RATE = 0.02
CHECKPOINTS = 10
TIME_LIMIT = 10


def read_items():
    numbers = list(map(int, sys.stdin.read().split()))
    capacity, count = numbers[0], numbers[1]
    return capacity, numbers[2:2 + 2 * count:2], numbers[3:3 + 2 * count:2]


class Knapsack:
    def __init__(self, capacity, weights, values):
        self.capacity = capacity
        self.weights = weights
        self.values = values
        self.worst_first = sorted(range(len(weights)), key=self.ratio)

    def ratio(self, item):
        return self.values[item] / self.weights[item] if self.weights[item] else inf

    def repair(self, genes):
        weight = sum(compress(self.weights, genes))
        for item in self.worst_first:
            if weight <= self.capacity:
                break
            if genes[item]:
                genes[item] = 0
                weight -= self.weights[item]
        return sum(compress(self.values, genes)), genes


def crossover(first, second):
    cut = random.randint(0, len(first))
    return first[:cut] + second[cut:]


def mutate(genes):
    for item in random.sample(range(len(genes)), max(1, round(len(genes) * MUTATION_RATE))):
        genes[item] ^= 1
    return genes


def select(population):
    return max(random.sample(population, TOURNAMENT_SIZE))[1]


def evolve(knapsack):
    n = len(knapsack.weights)
    deadline = time.perf_counter() + TIME_LIMIT
    population = sorted((knapsack.repair([random.randint(0, 1) for _ in range(n)]) for _ in range(POPULATION_SIZE)),
                        reverse=True)
    history = [population[0][0]]

    for _ in range(GENERATIONS):
        if time.perf_counter() > deadline:
            break
        next_generation = population[:ELITE_SIZE]
        while len(next_generation) < POPULATION_SIZE:
            child = mutate(crossover(select(population), select(population)))
            next_generation.append(knapsack.repair(child))

        population = sorted(next_generation, reverse=True)
        history.append(population[0][0])

    return history


def checkpoints(history):
    return [history[round(step * (len(history) - 1) / (CHECKPOINTS - 1))] for step in range(CHECKPOINTS)]


def main():
    knapsack = Knapsack(*read_items())

    start = time.perf_counter()
    history = evolve(knapsack)
    print(f"# TIMES_MS: alg={(time.perf_counter() - start) * 1000:.3f}")

    if os.environ.get("FMI_TIME_ONLY") == "1":
        return
    print(*checkpoints(history), sep="\n")
    print()
    print(history[-1])


if __name__ == "__main__":
    main()
