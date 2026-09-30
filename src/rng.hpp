#pragma once

#include <random>

namespace game {

struct Rng {
    std::mt19937 generator;

    Rng() : generator(std::random_device{}()) {}

    explicit Rng(unsigned seed) : generator(seed) {}

    float nextFloat(float minValue, float maxValue) {
        std::uniform_real_distribution<float> distribution(minValue, maxValue);
        return distribution(generator);
    }

    int nextInt(int minValue, int maxValue) {
        std::uniform_int_distribution<int> distribution(minValue, maxValue);
        return distribution(generator);
    }
};

}
