#include <cmath>
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>

const int POPULATION_SIZE = 200;
const int GENERATIONS = 500;
const int TOURNAMENT_SIZE = 3;
const int ELITE_SIZE = 2;
const double MUTATION_RATE = 0.02;
const int CHECKPOINTS = 10;
const double TIME_LIMIT_MS = 10000;

struct Item {
public:
    int weight = 0;
    int value = 0;

    Item(const int weight, const int value) {
        this->weight = weight;
        this->value = value;
    }

    double ratio() const {
        return weight == 0 ? INFINITY : static_cast<double>(value) / weight;
    }
};

struct Individual {
public:
    std::vector<int> genes;
    int value = 0;

    Individual(const std::vector<int>& genes, const int value) {
        this->genes = genes;
        this->value = value;
    }

    bool operator < (const Individual& other) const {
        return this->value > other.value;
    }
};

Individual repair(std::vector<int> genes, const std::vector<Item>& items,
    const std::vector<int>& worstFirst, const int capacity) {
    int weight = 0;
    for (size_t i = 0; i < items.size(); i++) {
        weight += genes[i] * items[i].weight;
    }

    for (const int currentItem: worstFirst) {
        if (weight <= capacity) {
            break;
        }
        if (genes[currentItem]) {
            genes[currentItem] = 0;
            weight -= items[currentItem].weight;
        }
    }

    int value = 0;
    for (size_t i = 0; i < items.size(); i++) {
        value += genes[i] * items[i].value;
    }
    return Individual(genes, value);
}

std::vector<int> onePointCrossover(const std::vector<int>& firstParent,
    const std::vector<int>& secondParent, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> distribution(0, firstParent.size());
    size_t point = distribution(rng);
    std::vector<int> child(firstParent.begin(), firstParent.begin() + point);
    child.insert(child.end(), secondParent.begin() + point, secondParent.end());
    return child;
}

void bitFlipMutation(std::vector<int>& genes, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> distribution(0, genes.size() - 1);
    int flips = std::max(1, static_cast<int>(std::lround(genes.size() * MUTATION_RATE)));
    for (int i = 0; i < flips; i++) {
        size_t position = distribution(rng);
        genes[position] = 1 - genes[position];
    }
}

const std::vector<int>& tournamentSelection(const std::vector<Individual>& population, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> distribution(0, population.size() - 1);
    size_t bestIndex = distribution(rng);
    for (int i = 1; i < TOURNAMENT_SIZE; i++) {
        size_t currentIndex = distribution(rng);
        if (population[currentIndex] < population[bestIndex]) {
            bestIndex = currentIndex;
        }
    }
    return population[bestIndex].genes;
}

std::vector<int> geneticAlgorithm(const std::vector<Item>& items, const int capacity, std::mt19937& rng) {
    auto start = std::chrono::steady_clock::now();
    std::vector<int> worstFirst(items.size());
    for (size_t i = 0; i < items.size(); i++) {
        worstFirst[i] = i;
    }
    std::sort(worstFirst.begin(), worstFirst.end(), [&items](const int first, const int second) {
        return items[first].ratio() < items[second].ratio();
    });

    std::uniform_int_distribution<int> bit(0, 1);
    std::vector<Individual> population;
    for (int i = 0; i < POPULATION_SIZE; i++) {
        std::vector<int> genes(items.size());
        for (int& currentGene: genes) {
            currentGene = bit(rng);
        }
        population.push_back(repair(genes, items, worstFirst, capacity));
    }
    std::sort(population.begin(), population.end());

    std::vector<int> history;
    history.push_back(population.front().value);
    for (int generation = 0; generation < GENERATIONS; generation++) {
        std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed.count() > TIME_LIMIT_MS) {
            break;
        }

        std::vector<Individual> nextPopulation(population.begin(), population.begin() + ELITE_SIZE);
        while (nextPopulation.size() < POPULATION_SIZE) {
            std::vector<int> child = onePointCrossover(tournamentSelection(population, rng),
                tournamentSelection(population, rng), rng);
            bitFlipMutation(child, rng);
            nextPopulation.push_back(repair(child, items, worstFirst, capacity));
        }

        std::sort(nextPopulation.begin(), nextPopulation.end());
        population.swap(nextPopulation);
        history.push_back(population.front().value);
    }

    return history;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int capacity = 0;
    int count = 0;
    std::cin >> capacity >> count;
    std::vector<Item> items;
    for (int i = 0; i < count; i++) {
        int weight = 0;
        int value = 0;
        std::cin >> weight >> value;
        items.push_back(Item(weight, value));
    }

    std::mt19937 rng(std::random_device{}());
    auto start = std::chrono::steady_clock::now();
    std::vector<int> history = geneticAlgorithm(items, capacity, rng);
    std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "# TIMES_MS: alg=" << elapsed.count() << '\n';

    const char* timeOnly = std::getenv("FMI_TIME_ONLY");
    if (timeOnly != nullptr && std::string(timeOnly) == "1") {
        return 0;
    }

    for (int step = 0; step < CHECKPOINTS; step++) {
        std::cout << history[std::lround(step * (history.size() - 1) / double(CHECKPOINTS - 1))] << '\n';
    }
    std::cout << '\n' << history.back() << '\n';

    return 0;
}
