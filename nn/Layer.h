#pragma once
// ============================================================================
// EvoSim — Neural Network Layer
// Single fully-connected layer with weights, biases, and activation
// ============================================================================

#include "Matrix.h"

namespace evo {

class Layer {
public:
    Mat weights;
    Vec biases;
    Vec lastInput;   // cached for backprop
    Vec lastOutput;  // cached for backprop
    Vec lastRaw;     // pre-activation values

    Layer() = default;

    Layer(int inputSize, int outputSize) {
        weights = Mat(outputSize, inputSize);
        biases = Vec(outputSize, 0.0f);
        weights.xavierInit();
        // Small random bias init
        auto& rng = Random::instance();
        for (int i = 0; i < outputSize; i++)
            biases[i] = rng.uniformFloat(-0.1f, 0.1f);
    }

    // Forward pass with ReLU activation
    Vec forward(const Vec& input) {
        lastInput = input;
        lastRaw = weights.multiply(input) + biases;
        lastOutput = lastRaw.apply(activations::relu);
        return lastOutput;
    }

    // Forward pass WITHOUT activation (for output layer — softmax applied externally)
    Vec forwardRaw(const Vec& input) {
        lastInput = input;
        lastRaw = weights.multiply(input) + biases;
        lastOutput = lastRaw; // No activation
        return lastOutput;
    }

    int inputSize() const { return weights.cols(); }
    int outputSize() const { return weights.rows(); }
};

} // namespace evo
