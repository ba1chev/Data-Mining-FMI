#include <vector>
#include <iostream>

std::vector<int> onePointCrossover(const std::vector<int>& firstParent,
    const std::vector<int>& secondParent, const size_t point) {
    std::vector<int> child(firstParent.begin(), firstParent.begin() + point);
    child.insert(child.end(), secondParent.begin() + point, secondParent.end());
    return child;
}

std::vector<int> twoPointCrossover(const std::vector<int>& firstParent,
    const std::vector<int>& secondParent, const size_t firstPoint, const size_t secondPoint) {
    std::vector<int> child(firstParent.begin(), firstParent.begin() + firstPoint);
    child.insert(child.end(), secondParent.begin() + firstPoint, secondParent.begin() + secondPoint);
    child.insert(child.end(), firstParent.begin() + secondPoint, firstParent.end());
    return child;
}

std::vector<int> uniformCrossover(const std::vector<int>& firstParent,
    const std::vector<int>& secondParent, const std::vector<bool>& mask) {
    std::vector<int> child;
    for (size_t i = 0; i < firstParent.size(); i++) {
        child.push_back(mask[i] ? firstParent[i] : secondParent[i]);
    }
    return child;
}
