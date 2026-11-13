#include <cmath>
#include <random>
#include <vector>
#include <iostream>
#include <algorithm>

typedef std::vector<int> Chromosome;
typedef double (*FitnessFunction)(const Chromosome&);
typedef Chromosome (*RandomIndividualFunction)(std::mt19937&);
typedef Chromosome (*CrossoverFunction)(const Chromosome&, const Chromosome&, std::mt19937&);
typedef Chromosome (*MutationFunction)(const Chromosome&, std::mt19937&);

struct Individual {
public:
    Chromosome chromosome;
    double fitness = 0;

    Individual(const Chromosome& chromosome, const double fitness) {
        this->chromosome = chromosome;
        this->fitness = fitness;
    }

    bool operator < (const Individual& other) const {
        return this->fitness > other.fitness;
    }
};

const Chromosome& tournamentSelection(const std::vector<Individual>& population,
    const int tournamentSize, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> distribution(0, population.size() - 1);
    size_t bestIndex = distribution(rng);
    for (int i = 1; i < tournamentSize; i++) {
        size_t currentIndex = distribution(rng);
        if (population[currentIndex].fitness > population[bestIndex].fitness) {
            bestIndex = currentIndex;
        }
    }
    return population[bestIndex].chromosome;
}

std::pair<Chromosome, std::vector<double>> geneticAlgorithm(RandomIndividualFunction randomIndividual,
    FitnessFunction fitness, CrossoverFunction crossover, MutationFunction mutate, std::mt19937& rng,
    const int populationSize = 100, const int generations = 500, const int elites = 2,
    const int tournamentSize = 3, const double targetFitness = INFINITY) {
    std::vector<Individual> population;
    for (int i = 0; i < populationSize; i++) {
        Chromosome currentChromosome = randomIndividual(rng);
        population.push_back(Individual(currentChromosome, fitness(currentChromosome)));
    }
    std::sort(population.begin(), population.end());

    std::vector<double> history;
    history.push_back(population.front().fitness);
    for (int generation = 0; generation < generations; generation++) {
        if (population.front().fitness >= targetFitness) {
            break;
        }

        std::vector<Individual> nextPopulation(population.begin(), population.begin() + elites);
        while (nextPopulation.size() < static_cast<size_t>(populationSize)) {
            const Chromosome& firstParent = tournamentSelection(population, tournamentSize, rng);
            const Chromosome& secondParent = tournamentSelection(population, tournamentSize, rng);
            Chromosome child = mutate(crossover(firstParent, secondParent, rng), rng);
            nextPopulation.push_back(Individual(child, fitness(child)));
        }

        std::sort(nextPopulation.begin(), nextPopulation.end());
        population.swap(nextPopulation);
        history.push_back(population.front().fitness);
    }

    return {population.front().chromosome, history};
}
