#pragma once
// ============================================================================
// EvoSim — Random Utility
// Thread-safe random number generation using Mersenne Twister
// ============================================================================

#include <random>
#include <cmath>

namespace evo {

class Random {
public:
    // Singleton access
    static Random& instance() {
        static Random rng;
        return rng;
    }

    // Uniform float in [min, max]
    float uniformFloat(float min = 0.0f, float max = 1.0f) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(engine_);
    }

    // Uniform int in [min, max]
    int uniformInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine_);
    }

    // Gaussian (normal) distribution
    float gaussian(float mean = 0.0f, float stddev = 1.0f) {
        std::normal_distribution<float> dist(mean, stddev);
        return dist(engine_);
    }

    // Boolean with probability p
    bool chance(float p) {
        return uniformFloat() < p;
    }

    // Seed the engine (for reproducibility)
    void seed(unsigned int s) {
        engine_.seed(s);
    }

private:
    Random() : engine_(std::random_device{}()) {}
    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;

    std::mt19937 engine_;
};

} // namespace evo
