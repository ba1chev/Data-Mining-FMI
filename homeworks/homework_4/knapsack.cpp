#include <cmath>
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>

// Parameters of the genetic algorithm:
// 1) chromosomes in every generation
// 2) generations to evolve
// 3) chromosomes competing in every tournament
// 4) best chromosomes copied unchanged to the next generation
// 5) part of the genes flipped by every mutation
// 6) values printed from the history
namespace Parameters {
    const size_t POPULATION_SIZE = 200;
    const size_t GENERATIONS = 500;
    const size_t TOURNAMENT_SIZE = 3;
    const size_t ELITE_SIZE = 2;
    const double MUTATION_RATE = 0.02;
    const size_t CHECKPOINTS = 10;
}

// Defining an abstraction
// for the genes
template <class T>
struct Gene {
public:
    T data = T{};
};

// Defining an abstraction for the
// chromosomes, the fitness is cached
// so it is computed only once
template <class T>
struct Chromosome {
public:
    std::vector<Gene<T>> genes;
    size_t fitness = 0;
};

// Defining function that creates
// a random chromosome for the problem
template <class T, class P>
using GeneratorFunction =
    Chromosome<T> (*)(const P&, std::mt19937&);

// Defining function that measures
// how good a chromosome is
template <class T, class P>
using FitnessFunction =
    size_t (*)(const Chromosome<T>&, const P&);

// Defining function that turns an invalid
// chromosome into a valid one
template <class T, class P>
using RepairFunction =
    void (*)(Chromosome<T>&, const P&);

template <class T>
using CrossoverFunction =
    Chromosome<T> (*)(const Chromosome<T>&, const Chromosome<T>&, std::mt19937&);

template <class T>
using MutationFunction =
    void (*)(Chromosome<T>&, std::mt19937&);

struct Item {
public:
    size_t weight = 0;
    size_t value = 0;

    // An item without weight is
    // always worth taking
    double ratio() const {
        double result = value;
        return weight == 0 ? INFINITY : result / weight;
    }
};

struct Knapsack {
public:
    size_t capacity = 0;
    std::vector<Item> items;
    // Indices of the items from the best
    // to the worst value per weight
    std::vector<size_t> byRatio;
};

template <class T, class P>
class GeneticAlgorithm {
private:
    const P& problem;
    GeneratorFunction<T, P> generator;
    FitnessFunction<T, P> fitness;
    RepairFunction<T, P> repair;
    CrossoverFunction<T> crossover;
    MutationFunction<T> mutation;
    std::vector<Chromosome<T>> population;

    void evaluate(Chromosome<T>& chromosome) const {
        repair(chromosome, problem);
        chromosome.fitness = fitness(chromosome, problem);
    }

    void sortPopulation() {
        std::sort(population.begin(), population.end(),
            [](const Chromosome<T>& first, const Chromosome<T>& second) {
                return first.fitness > second.fitness;
            });
    }

    void initialize(std::mt19937& rng) {
        population.clear();
        for (size_t i = 0; i < Parameters::POPULATION_SIZE; i++) {
            Chromosome<T> chromosome = generator(problem, rng);
            evaluate(chromosome);
            population.push_back(chromosome);
        }
        sortPopulation();
    }

    const Chromosome<T>& tournamentSelection(std::mt19937& rng) const {
        std::uniform_int_distribution<size_t> distribution(0, population.size() - 1);
        size_t best = distribution(rng);
        for (size_t i = 1; i < Parameters::TOURNAMENT_SIZE; i++) {
            size_t current = distribution(rng);
            if (population[current].fitness > population[best].fitness) {
                best = current;
            }
        }

        return population[best];
    }

    void nextGeneration(std::mt19937& rng) {
        // Elitism: the best chromosomes survive unchanged,
        // so the best fitness never decreases
        std::vector<Chromosome<T>> next(population.begin(),
            population.begin() + std::min(Parameters::ELITE_SIZE, population.size()));
        while (next.size() < Parameters::POPULATION_SIZE) {
            Chromosome<T> child = crossover(tournamentSelection(rng), tournamentSelection(rng), rng);
            mutation(child, rng);
            evaluate(child);
            next.push_back(child);
        }

        population.swap(next);
        sortPopulation();
    }

public:
    GeneticAlgorithm(const P& problem, const GeneratorFunction<T, P> generator,
        const FitnessFunction<T, P> fitness, const RepairFunction<T, P> repair,
        const CrossoverFunction<T> crossover, const MutationFunction<T> mutation)
        : problem(problem) {
        this->generator = generator;
        this->fitness = fitness;
        this->repair = repair;
        this->crossover = crossover;
        this->mutation = mutation;
    }

    std::vector<size_t> run(const size_t generations, std::mt19937& rng) {
        initialize(rng);
        std::vector<size_t> history = {population.front().fitness};
        for (size_t generation = 0; generation < generations; generation++) {
            nextGeneration(rng);
            history.push_back(population.front().fitness);
        }

        return history;
    }
};

Chromosome<bool> generateKnapsackChromosome(const Knapsack& knapsack, std::mt19937& rng) {
    std::bernoulli_distribution coin(0.5);
    Chromosome<bool> result;
    result.genes.resize(knapsack.items.size());
    for (Gene<bool>& gene: result.genes) {
        gene.data = coin(rng);
    }

    return result;
}

size_t knapsackFitness(const Chromosome<bool>& chromosome, const Knapsack& knapsack) {
    // Multiplying instead of branching,
    // the genes are random and the
    // branch is mispredicted half the time
    size_t weight = 0;
    size_t value = 0;
    for (size_t i = 0; i < chromosome.genes.size(); i++) {
        weight += chromosome.genes[i].data * knapsack.items[i].weight;
        value += chromosome.genes[i].data * knapsack.items[i].value;
    }

    return weight <= knapsack.capacity ? value : 0;
}

// While too heavy, drop the taken
// items with the worst value per weight
void knapsackRepair(Chromosome<bool>& chromosome, const Knapsack& knapsack) {
    size_t weight = 0;
    for (size_t i = 0; i < chromosome.genes.size(); i++) {
        weight += chromosome.genes[i].data * knapsack.items[i].weight;
    }

    for (auto it = knapsack.byRatio.rbegin(); it != knapsack.byRatio.rend() && weight > knapsack.capacity; it++) {
        if (chromosome.genes[*it].data) {
            chromosome.genes[*it].data = false;
            weight -= knapsack.items[*it].weight;
        }
    }
}

// The cut point can also be at both ends,
// then the child is a copy of one parent
template <class T>
Chromosome<T> onePointCrossover(const Chromosome<T>& first,
    const Chromosome<T>& second, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> distribution(0, first.genes.size());
    size_t point = distribution(rng);

    Chromosome<T> child;
    child.genes.assign(first.genes.begin(), first.genes.begin() + point);
    child.genes.insert(child.genes.end(), second.genes.begin() + point, second.genes.end());
    return child;
}

// At least one gene is flipped,
// so every child differs from its parents
void bitFlipMutation(Chromosome<bool>& chromosome, std::mt19937& rng) {
    if (chromosome.genes.empty()) {
        return;
    }

    std::uniform_int_distribution<size_t> distribution(0, chromosome.genes.size() - 1);
    size_t flips = std::max<size_t>(1, std::lround(chromosome.genes.size() * Parameters::MUTATION_RATE));
    for (size_t i = 0; i < flips; i++) {
        Gene<bool>& gene = chromosome.genes[distribution(rng)];
        gene.data = !gene.data;
    }
}

Knapsack readKnapsack() {
    Knapsack knapsack;
    size_t count = 0;
    std::cin >> knapsack.capacity >> count;

    knapsack.items.resize(count);
    for (Item& item: knapsack.items) {
        std::cin >> item.weight >> item.value;
    }

    knapsack.byRatio.resize(count);
    for (size_t i = 0; i < count; i++) {
        knapsack.byRatio[i] = i;
    }
    std::sort(knapsack.byRatio.begin(), knapsack.byRatio.end(),
        [&knapsack](const size_t first, const size_t second) {
            return knapsack.items[first].ratio() > knapsack.items[second].ratio();
        });

    return knapsack;
}

// Parameters::CHECKPOINTS values evenly spread over the history:
// 1) the first one is the initial population
// 2) the last one is the final generation
void printHistory(const std::vector<size_t>& history) {
    size_t last = history.size() - 1;
    size_t steps = Parameters::CHECKPOINTS - 1;
    for (size_t step = 0; step <= steps; step++) {
        std::cout << history[(step * last + steps / 2) / steps] << '\n';
    }
    std::cout << '\n' << history.back() << '\n';
}

void printTime(const std::chrono::steady_clock::time_point& start) {
    std::chrono::duration<double, std::milli> elapsed =
        std::chrono::steady_clock::now() - start;
    std::cout << "# TIMES_MS: alg=" << elapsed.count() << '\n';
}

bool isTimeOnly() {
    const char* timeOnly = std::getenv("FMI_TIME_ONLY");
    return timeOnly != nullptr && std::string(timeOnly) == "1";
}

int main() {
    Knapsack knapsack = readKnapsack();
    std::mt19937 rng(std::random_device{}());
    GeneticAlgorithm<bool, Knapsack> geneticAlgorithm(knapsack, generateKnapsackChromosome,
        knapsackFitness, knapsackRepair, onePointCrossover<bool>, bitFlipMutation);

    auto start = std::chrono::steady_clock::now();
    std::vector<size_t> history = geneticAlgorithm.run(Parameters::GENERATIONS, rng);
    printTime(start);

    if (isTimeOnly()) {
        return 0;
    }

    printHistory(history);
    return 0;
}
