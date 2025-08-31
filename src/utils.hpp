#pragma once
#include <random>
#include <iostream>

[[maybe_unused]] static float get_random_float() {
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_real_distribution<float> distribution{0.0f, 1.0f};
    return distribution(generator);
}

[[maybe_unused]] static float get_random_float_normal(float m, float sigma) {
    static std::mt19937 generator{std::random_device{}()};
    std::normal_distribution<float> distribution{m, sigma};
    return distribution(generator);
}

[[maybe_unused]] static int get_random_int(int max) {
    return static_cast<int>(max * get_random_float());
}