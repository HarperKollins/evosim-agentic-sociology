#pragma once
// ============================================================================
// EvoSim — Neural Network (The Brain)
// Feedforward NN with ReLU hidden layers, Softmax output
// Supports: forward pass, backprop (Q-learning), Gaussian mutation
// Architecture: [inputs] → [16] → [16] → [outputs]
// ============================================================================

#include "Layer.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>

namespace evo {

class NeuralNetwork {
public:
    std::vector<Layer> layers;
    float learningRate = 0.1f; // "Self-Awareness" plasticity parameter

    NeuralNetwork() = default;

    // Build network with given architecture
    // e.g., {6, 16, 16, 8} = 6 inputs, two hidden layers of 16, 8 outputs
    explicit NeuralNetwork(const std::vector<int>& architecture) {
        for (size_t i = 0; i + 1 < architecture.size(); i++) {
            layers.emplace_back(architecture[i], architecture[i + 1]);
        }
    }

    // ── Forward Pass ────────────────────────────────────────────────────────
    // Returns raw Q-values (no softmax) for RL action selection
    Vec getQValues(const Vec& state) const {
        Vec current = state;
        for (size_t i = 0; i < layers.size(); i++) {
            if (i < layers.size() - 1) {
                // Hidden layers: ReLU
                current = const_cast<Layer&>(layers[i]).forward(current);
            } else {
                // Output layer: raw values (Q-values)
                current = const_cast<Layer&>(layers[i]).forwardRaw(current);
            }
        }
        return current;
    }

    Vec feedForward(const Vec& input) const {
        return getQValues(input);
    }

    // Returns softmax probabilities (for action selection)
    Vec getActionProbabilities(const Vec& state) const {
        Vec qValues = getQValues(state);
        return activations::softmax(qValues);
    }

    // ── Backpropagation (Simple Q-Learning Update) ──────────────────────────
    // Updates weights to move Q(s,a) toward target value
    void backpropagate(const Vec& state, int action, float target, float lr) {
        // Forward pass to populate cached values
        Vec qValues = getQValues(state);

        // Compute output error: only for the selected action
        Vec outputError(qValues.size(), 0.0f);
        outputError[action] = target - qValues[action];

        // Backprop through layers (reverse order)
        Vec delta = outputError;

        for (int l = static_cast<int>(layers.size()) - 1; l >= 0; l--) {
            Layer& layer = layers[l];

            // For hidden layers, apply ReLU derivative
            if (l < static_cast<int>(layers.size()) - 1) {
                for (int i = 0; i < delta.size(); i++) {
                    delta[i] *= activations::reluDeriv(layer.lastRaw[i]);
                }
            }

            // Compute weight gradients and update
            for (int i = 0; i < layer.outputSize(); i++) {
                for (int j = 0; j < layer.inputSize(); j++) {
                    layer.weights.at(i, j) += lr * delta[i] * layer.lastInput[j];
                }
                layer.biases[i] += lr * delta[i];
            }

            // Propagate error to previous layer
            if (l > 0) {
                Vec prevDelta(layer.inputSize(), 0.0f);
                for (int j = 0; j < layer.inputSize(); j++) {
                    float sum = 0.0f;
                    for (int i = 0; i < layer.outputSize(); i++) {
                        sum += layer.weights.at(i, j) * delta[i];
                    }
                    prevDelta[j] = sum;
                }
                delta = prevDelta;
            }
        }
    }

    void train(const Vec& inputs, int action, float reward) {
        backpropagate(inputs, action, reward, learningRate);
    }

    // ── Gaussian Mutation (Evolution) ───────────────────────────────────────
    // sigma: std deviation of noise
    // perturbRate: fraction of weights to perturb (0.0 to 1.0)
    // Formula: W_new = W_old + N(0, σ)  (only for selected weights)
    void mutate(float sigma = 0.05f, float perturbRate = 0.8f) {
        for (auto& layer : layers) {
            layer.weights.mutateGaussian(sigma, perturbRate);
            // Also mutate biases
            auto& rng = Random::instance();
            for (int i = 0; i < layer.biases.size(); i++) {
                if (rng.chance(perturbRate)) {
                    layer.biases[i] += rng.gaussian(0.0f, sigma);
                }
            }
        }
    }

    // ── Deep Copy ───────────────────────────────────────────────────────────
    NeuralNetwork clone() const {
        NeuralNetwork copy;
        copy.layers = layers; // vector of Layer, default copy
        return copy;
    }

    // Get total parameter count
    int parameterCount() const {
        int count = 0;
        for (const auto& layer : layers) {
            count += layer.weights.totalElements() + layer.biases.size();
        }
        return count;
    }

    static NeuralNetwork crossover(const NeuralNetwork& parent1, const NeuralNetwork& parent2) {
        NeuralNetwork child = parent1.clone();
        auto& rng = Random::instance();
        
        for (size_t l = 0; l < child.layers.size(); l++) {
            auto& layer = child.layers[l];
            const auto& p2Layer = parent2.layers[l];
            
            for (int i = 0; i < layer.outputSize(); i++) {
                for (int j = 0; j < layer.inputSize(); j++) {
                    if (rng.chance(0.5f)) {
                        layer.weights.at(i, j) = p2Layer.weights.at(i, j);
                    }
                }
                if (rng.chance(0.5f)) {
                    layer.biases[i] = p2Layer.biases[i];
                }
            }
        }
        return child;
    }

    // ── Persistence ─────────────────────────────────────────────────────────
    bool save(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        
        file << layers.size() << "\n";
        for (const auto& layer : layers) {
            file << layer.inputSize() << " " << layer.outputSize() << "\n";
            // Weights
            for (int i = 0; i < layer.outputSize(); i++) {
                for (int j = 0; j < layer.inputSize(); j++) {
                    file << layer.weights.at(i, j) << " ";
                }
                file << "\n";
            }
            // Biases
            for (size_t k = 0; k < layer.biases.size(); k++) {
                file << layer.biases[k] << " ";
            }
            file << "\n";
        }
        return true;
    }

    bool load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        int layerCount;
        file >> layerCount;
        layers.clear();

        for (int l = 0; l < layerCount; l++) {
            int inSize, outSize;
            file >> inSize >> outSize;
            layers.emplace_back(inSize, outSize);
            Layer& layer = layers.back();

            // Weights
            for (int i = 0; i < outSize; i++) {
                for (int j = 0; j < inSize; j++) {
                    file >> layer.weights.at(i, j);
                }
            }
            // Biases
            for (int i = 0; i < outSize; i++) {
                file >> layer.biases[i];
            }
        }
        return true;
    }
};

} // namespace evo
