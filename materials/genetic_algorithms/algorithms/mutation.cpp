#include <random>
#include <vector>
#include <iostream>
#include <algorithm>

std::vector<int> bitFlipMutation(const std::vector<int>& chromosome, const std::vector<size_t>& positions) {
    std::vector<int> mutated = chromosome;
    for (const size_t currentPosition: positions) {
        mutated[currentPosition] = 1 - mutated[currentPosition];
    }
    return mutated;
}

std::vector<int> swapMutation(const std::vector<int>& chromosome, const size_t i, const size_t j) {
    std::vector<int> mutated = chromosome;
    std::swap(mutated[i], mutated[j]);
    return mutated;
}

std::vector<int> insertionMutation(const std::vector<int>& chromosome, const size_t i, const size_t j) {
    std::vector<int> mutated = chromosome;
    int gene = mutated[j];
    mutated.erase(mutated.begin() + j);
    mutated.insert(mutated.begin() + i + 1, gene);
    return mutated;
}

std::vector<int> reverseMutation(const std::vector<int>& chromosome, const size_t i, const size_t j) {
    std::vector<int> mutated = chromosome;
    std::reverse(mutated.begin() + i, mutated.begin() + j + 1);
    return mutated;
}

std::vector<int> scrambleMutation(const std::vector<int>& chromosome,
    const size_t i, const size_t j, std::mt19937& rng) {
    std::vector<int> mutated = chromosome;
    std::shuffle(mutated.begin() + i, mutated.begin() + j + 1, rng);
    return mutated;
}

std::vector<double> gaussianMutation(const std::vector<double>& chromosome,
    const double sigma, std::mt19937& rng) {
    std::normal_distribution<double> distribution(0.0, sigma);
    std::vector<double> mutated = chromosome;
    for (double& currentGene: mutated) {
        currentGene += distribution(rng);
    }
    return mutated;
}
