#pragma once
// ============================================================================
// EvoSim — Matrix Operations
// Lightweight matrix/vector types for neural network computation
// No external dependencies — pure C++17
// ============================================================================

#include <vector>
#include <cmath>
#include <cassert>
#include <iostream>
#include <functional>
#include "../utils/Random.h"

namespace evo {

// ── Vector Type ─────────────────────────────────────────────────────────────
class Vec {
public:
    std::vector<float> data;

    Vec() = default;
    explicit Vec(int size, float val = 0.0f) : data(size, val) {}
    Vec(const std::vector<float>& d) : data(d) {}
    Vec(std::initializer_list<float> init) : data(init) {}

    int size() const { return static_cast<int>(data.size()); }

    float& operator[](int i) { return data[i]; }
    float  operator[](int i) const { return data[i]; }

    // Element-wise operations
    Vec operator+(const Vec& other) const {
        assert(size() == other.size());
        Vec result(size());
        for (int i = 0; i < size(); i++)
            result[i] = data[i] + other[i];
        return result;
    }

    Vec operator-(const Vec& other) const {
        assert(size() == other.size());
        Vec result(size());
        for (int i = 0; i < size(); i++)
            result[i] = data[i] - other[i];
        return result;
    }

    Vec operator*(float scalar) const {
        Vec result(size());
        for (int i = 0; i < size(); i++)
            result[i] = data[i] * scalar;
        return result;
    }

    // Dot product
    float dot(const Vec& other) const {
        assert(size() == other.size());
        float sum = 0.0f;
        for (int i = 0; i < size(); i++)
            sum += data[i] * other[i];
        return sum;
    }

    // Apply function element-wise
    Vec apply(std::function<float(float)> fn) const {
        Vec result(size());
        for (int i = 0; i < size(); i++)
            result[i] = fn(data[i]);
        return result;
    }

    // Max element index
    int argmax() const {
        int best = 0;
        for (int i = 1; i < size(); i++)
            if (data[i] > data[best]) best = i;
        return best;
    }

    // Max value
    float max() const {
        float m = data[0];
        for (int i = 1; i < size(); i++)
            if (data[i] > m) m = data[i];
        return m;
    }
};

// ── Matrix Type ─────────────────────────────────────────────────────────────
class Mat {
public:
    std::vector<float> data;
    int rows_, cols_;

    Mat() : rows_(0), cols_(0) {}
    Mat(int rows, int cols, float val = 0.0f)
        : data(rows * cols, val), rows_(rows), cols_(cols) {}

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    float& at(int r, int c) { return data[r * cols_ + c]; }
    float  at(int r, int c) const { return data[r * cols_ + c]; }

    // Matrix-vector multiply: result = M * v
    Vec multiply(const Vec& v) const {
        assert(cols_ == v.size());
        Vec result(rows_);
        for (int i = 0; i < rows_; i++) {
            float sum = 0.0f;
            for (int j = 0; j < cols_; j++)
                sum += at(i, j) * v[j];
            result[i] = sum;
        }
        return result;
    }

    // Xavier initialization
    void xavierInit() {
        float limit = std::sqrt(6.0f / (rows_ + cols_));
        auto& rng = Random::instance();
        for (auto& w : data)
            w = rng.uniformFloat(-limit, limit);
    }

    // Gaussian mutation with weight mask
    void mutateGaussian(float sigma, float perturbRate) {
        auto& rng = Random::instance();
        for (auto& w : data) {
            if (rng.chance(perturbRate)) {
                w += rng.gaussian(0.0f, sigma);
            }
        }
    }

    int totalElements() const { return rows_ * cols_; }
};

// ── Activation Functions ────────────────────────────────────────────────────
namespace activations {

    inline float relu(float x) { return x > 0.0f ? x : 0.0f; }
    inline float reluDeriv(float x) { return x > 0.0f ? 1.0f : 0.0f; }

    inline Vec softmax(const Vec& v) {
        Vec result(v.size());
        float maxVal = v.max();
        float sum = 0.0f;
        for (int i = 0; i < v.size(); i++) {
            result[i] = std::exp(v[i] - maxVal); // numerical stability
            sum += result[i];
        }
        for (int i = 0; i < v.size(); i++)
            result[i] /= sum;
        return result;
    }

} // namespace activations

} // namespace evo
