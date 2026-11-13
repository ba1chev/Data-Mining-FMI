#include <set>
#include <cmath>
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <unordered_set>

const int POPULATION_SIZE = 300;
const int GENERATIONS = 1000;
const int TOURNAMENT_SIZE = 3;
const int ELITE_SIZE = 2;
const double MUTATION_RATE = 0.5;
const int CHECKPOINTS = 10;
const double PLANE_SIZE = 1000.0;

struct City {
public:
    std::string name;
    double x = 0;
    double y = 0;

    City(const std::string& name, const double x, const double y) {
        this->name = name;
        this->x = x;
        this->y = y;
    }
};

struct Individual {
public:
    std::vector<int> route;
    double length = 0;

    Individual(const std::vector<int>& route, const double length) {
        this->route = route;
        this->length = length;
    }

    bool operator < (const Individual& other) const {
        return this->length < other.length;
    }
};

double routeLength(const std::vector<int>& route, const std::vector<std::vector<double>>& distances) {
    double length = 0;
    for (size_t i = 1; i < route.size(); i++) {
        length += distances[route[i - 1]][route[i]];
    }
    return length;
}

std::pair<size_t, size_t> randomSegment(const size_t size, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> distribution(0, size);
    size_t first = distribution(rng);
    size_t second = distribution(rng);
    return {std::min(first, second), std::max(first, second)};
}

std::vector<int> orderCrossover(const std::vector<int>& firstParent,
    const std::vector<int>& secondParent, std::mt19937& rng) {
    std::pair<size_t, size_t> segment = randomSegment(firstParent.size(), rng);
    std::unordered_set<int> taken(firstParent.begin() + segment.first, firstParent.begin() + segment.second);

    std::vector<int> rest;
    for (const int currentCity: secondParent) {
        if (taken.find(currentCity) == taken.end()) {
            rest.push_back(currentCity);
        }
    }

    std::vector<int> child(rest.begin(), rest.begin() + segment.first);
    child.insert(child.end(), firstParent.begin() + segment.first, firstParent.begin() + segment.second);
    child.insert(child.end(), rest.begin() + segment.first, rest.end());
    return child;
}

void reverseMutation(std::vector<int>& route, std::mt19937& rng) {
    std::pair<size_t, size_t> segment = randomSegment(route.size(), rng);
    std::reverse(route.begin() + segment.first, route.begin() + segment.second);
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
    return population[bestIndex].route;
}

size_t populationSize(const size_t countOfCities) {
    size_t permutations = 1;
    for (size_t i = 2; i <= countOfCities && permutations < POPULATION_SIZE; i++) {
        permutations *= i;
    }
    return std::min(permutations, static_cast<size_t>(POPULATION_SIZE));
}

std::pair<std::vector<int>, std::vector<double>> geneticAlgorithm(const std::vector<City>& cities, std::mt19937& rng) {
    size_t n = cities.size();
    std::vector<std::vector<double>> distances(n, std::vector<double>(n));
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            distances[i][j] = std::hypot(cities[i].x - cities[j].x, cities[i].y - cities[j].y);
        }
    }

    size_t size = populationSize(n);
    std::vector<Individual> population;
    std::vector<int> route(n);
    for (size_t i = 0; i < n; i++) {
        route[i] = i;
    }
    for (size_t i = 0; i < size; i++) {
        std::shuffle(route.begin(), route.end(), rng);
        population.push_back(Individual(route, routeLength(route, distances)));
    }
    std::sort(population.begin(), population.end());

    std::uniform_real_distribution<double> chance(0.0, 1.0);
    std::vector<double> history;
    history.push_back(population.front().length);
    for (int generation = 0; generation < GENERATIONS; generation++) {
        std::vector<Individual> nextPopulation(population.begin(),
            population.begin() + std::min(static_cast<size_t>(ELITE_SIZE), size));
        std::set<std::vector<int>> seen;
        for (const Individual& currentIndividual: nextPopulation) {
            seen.insert(currentIndividual.route);
        }

        while (nextPopulation.size() < size) {
            std::vector<int> child = orderCrossover(tournamentSelection(population, rng),
                tournamentSelection(population, rng), rng);
            if (chance(rng) < MUTATION_RATE) {
                reverseMutation(child, rng);
            }
            if (seen.insert(child).second) {
                nextPopulation.push_back(Individual(child, routeLength(child, distances)));
            }
        }

        std::sort(nextPopulation.begin(), nextPopulation.end());
        population.swap(nextPopulation);
        history.push_back(population.front().length);
    }

    return {population.front().route, history};
}

std::vector<City> readCities(std::mt19937& rng, bool& isNamedDataset) {
    std::string firstToken;
    std::cin >> firstToken;

    std::vector<City> cities;
    isNamedDataset = !std::all_of(firstToken.begin(), firstToken.end(), ::isdigit);
    if (!isNamedDataset) {
        std::uniform_real_distribution<double> coordinate(0.0, PLANE_SIZE);
        for (int i = 0; i < std::stoi(firstToken); i++) {
            cities.push_back(City(std::to_string(i), coordinate(rng), coordinate(rng)));
        }
        return cities;
    }

    int count = 0;
    std::cin >> count;
    for (int i = 0; i < count; i++) {
        std::string name;
        double x = 0;
        double y = 0;
        std::cin >> name >> x >> y;
        cities.push_back(City(name, x, y));
    }
    return cities;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::mt19937 rng(std::random_device{}());
    bool isNamedDataset = false;
    std::vector<City> cities = readCities(rng, isNamedDataset);

    auto start = std::chrono::steady_clock::now();
    std::pair<std::vector<int>, std::vector<double>> result = geneticAlgorithm(cities, rng);
    std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "# TIMES_MS: alg=" << elapsed.count() << '\n';

    const char* timeOnly = std::getenv("FMI_TIME_ONLY");
    if (timeOnly != nullptr && std::string(timeOnly) == "1") {
        return 0;
    }

    const std::vector<double>& history = result.second;
    std::cout << std::setprecision(15);
    for (int step = 0; step < CHECKPOINTS; step++) {
        std::cout << history[std::lround(step * (history.size() - 1) / double(CHECKPOINTS - 1))] << '\n';
    }
    std::cout << '\n';

    if (isNamedDataset) {
        for (size_t i = 0; i < result.first.size(); i++) {
            std::cout << (i > 0 ? " -> " : "") << cities[result.first[i]].name;
        }
        std::cout << '\n';
    }
    std::cout << history.back() << '\n';

    return 0;
}
