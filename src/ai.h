#ifndef AI_H
#define AI_H

#include <vector>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>

using namespace std;

class Neuron {
private:
    vector<double> weights;
    double bias;

public:
    Neuron(unsigned numInputs) : bias(0.0) {
        weights.resize(numInputs);
    }

    void randomize() {
        for (double& weight : weights) {
            weight = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0;
        }
        bias = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0;
    }

    void mutate(double mutationRate) {
        for (double& weight : weights) {
            if ((static_cast<double>(rand()) / RAND_MAX) < mutationRate) {
                weight += (static_cast<double>(rand()) / RAND_MAX) * 0.2 - 0.1; 
                            if (weight > 1.0) weight = 1.0;
                if (weight < -1.0) weight = -1.0;
            }
        }
        if ((static_cast<double>(rand()) / RAND_MAX) < mutationRate) {
            bias += (static_cast<double>(rand()) / RAND_MAX) * 0.2 - 0.1; 
                    if (bias > 1.0) bias = 1.0;
            if (bias < -1.0) bias = -1.0;
        }
    }

    double result(const vector<double>& inputs) const {
            double sum = inner_product(inputs.begin(), inputs.end(), weights.begin(), 0.0);
        sum += bias;
            return tanh(sum);
    }

    friend class Network;

    ~Neuron() {
        // Destructor to clean up resources if needed
    }
    Neuron(const Neuron& other) : weights(other.weights), bias(other.bias) {
        // Copy constructor
    }
    Neuron& operator=(const Neuron& other) {
        if (this != &other) {
            weights = other.weights;
            bias = other.bias;
        }
        return *this;
    }
    Neuron(Neuron&& other) noexcept : weights(std::move(other.weights)), bias(other.bias) {
        // Move constructor
    }
    Neuron& operator=(Neuron&& other) noexcept {
        if (this != &other) {
            weights = std::move(other.weights);
            bias = other.bias;
        }
        return *this;
    }
    Neuron() = default; // Default constructor for Neuron
};


class Network {
private:
    vector<vector<Neuron>> hiddenLayers;
    vector<Neuron> outputLayer;

public:
    Network(unsigned inputCount, unsigned hiddenLayerCount, unsigned neuronsPerHiddenLayer, unsigned outputCount) {
            if (hiddenLayerCount > 0) {
                    unsigned prevLayerSize = inputCount;
            hiddenLayers.emplace_back();
            for (unsigned j = 0; j < neuronsPerHiddenLayer; ++j) {
                hiddenLayers.back().emplace_back(prevLayerSize);
            }
            prevLayerSize = neuronsPerHiddenLayer;

                    for (unsigned i = 1; i < hiddenLayerCount; ++i) {
                hiddenLayers.emplace_back();
                for (unsigned j = 0; j < neuronsPerHiddenLayer; ++j) {
                    hiddenLayers.back().emplace_back(prevLayerSize);
                }
            }
        }

            unsigned lastHiddenLayerSize = (hiddenLayerCount > 0) ? neuronsPerHiddenLayer : inputCount;
        for (unsigned i = 0; i < outputCount; ++i) {
            outputLayer.emplace_back(lastHiddenLayerSize);
        }
    }

    void randomize() {
        for (auto& layer : hiddenLayers) {
            for (auto& neuron : layer) {
                neuron.randomize();
            }
        }
        for (auto& neuron : outputLayer) {
            neuron.randomize();
        }
    }

    void mutate(double mutationRate) {
        for (auto& layer : hiddenLayers) {
            for (auto& neuron : layer) {
                neuron.mutate(mutationRate);
            }
        }
        for (auto& neuron : outputLayer) {
            neuron.mutate(mutationRate);
        }
    }

    vector<double> result(const vector<double>& inputs) const {
        vector<double> currentOutputs = inputs;

            for (const auto& layer : hiddenLayers) {
            vector<double> nextOutputs;
            nextOutputs.reserve(layer.size());
            for (const auto& neuron : layer) {
                nextOutputs.push_back(neuron.result(currentOutputs));
            }
            currentOutputs = nextOutputs;
        }

            vector<double> finalOutputs;
        finalOutputs.reserve(outputLayer.size());
        for (const auto& neuron : outputLayer) {
            finalOutputs.push_back(neuron.result(currentOutputs));
        }

        return finalOutputs;
    }
    double computeTotalError(const vector<vector<double>>& inputs, const vector<vector<double>>& expected) const {
        double totalError = 0.0;
        for (size_t i = 0; i < inputs.size(); ++i) {
            auto res = result(inputs[i]);
            for (size_t j = 0; j < res.size(); ++j) {
                totalError += pow(res[j] - expected[i][j], 2);
            }
        }
        return totalError / inputs.size(); 
    }

    void train(const vector<vector<double>>& inputs,
            const vector<vector<double>>& expected,
            double targetError,
            unsigned maxGenerations = 10000,
            double mutationRate = 0.1)
    {
        double bestError = computeTotalError(inputs, expected);
        cout << "Initial Error: " << bestError << endl;

        for (unsigned gen = 0; gen < maxGenerations && bestError > targetError; ++gen) {
                    Network backup = *this;

                    mutate(mutationRate);
            double currentError = computeTotalError(inputs, expected);

                    if (currentError < bestError) {
                bestError = currentError;
                if (gen % 1000 == 0) { 
                    cout << "Generation " << gen << ", New Best Error: " << bestError << endl;
                }
            } else {
                            *this = backup;
            }
        }
            cout << "------------------------------------" << endl;
        if (bestError <= targetError) {
            cout << "Target error reached! Final Error: " << bestError << endl;
        } else {
            cout << "Max generations reached. Final Error: " << bestError << endl;
        }
        cout << "------------------------------------" << endl;
    }

    ~Network() {
        
    }
    Network(const Network& other) : hiddenLayers(other.hiddenLayers), outputLayer(other.outputLayer) {
        // Copy constructor
    }
    Network& operator=(const Network& other) {
        if (this != &other) {
            hiddenLayers = other.hiddenLayers;
            outputLayer = other.outputLayer;
        }
        return *this;
    }
    Network(Network&& other) noexcept : hiddenLayers(std::move(other.hiddenLayers)), outputLayer(std::move(other.outputLayer)) {
        // Move constructor
    }
    Network& operator=(Network&& other) noexcept {
        if (this != &other) {
            hiddenLayers = std::move(other.hiddenLayers);
            outputLayer = std::move(other.outputLayer);
        }
        return *this;
    }
    Network() = default; // Default constructor for Network
};

class EnemyBrain {
private:
    Network brain = Network(3, 3, 3, 2); // Inputs: {health/100, distance to player, angle_to_player}, Outputs: {towards_player, angle_to_move}
    // daca output[0] > 0, mergi spre player, altfel, mergi in directia opusa
    // output[1] este un unghi de miscare, de la 0 la 360 grade
public:
    EnemyBrain() {
        brain.randomize();
    }

    void mutate(double mutationRate) {
        brain.mutate(mutationRate);
    }

    vector<double> result(double health, unsigned enemyX, unsigned enemyY, unsigned playerX, unsigned playerY, double angleToPlayer) {
        vector<double> inputs = {
            health / 100.0, // Normalize health to 0-1
            static_cast<double>(sqrt(pow(enemyX - playerX, 2) + pow(enemyY - playerY, 2))) / 600.0, // Distance to player normalized
            angleToPlayer / 360.0 // Normalize angle to 0-1
        };
        auto output = brain.result(inputs);
        bool towardsPlayer = output[0];
        double moveAngle = output[1] * 180.0 + 180.0; 
        return {towardsPlayer > 0 ? 1.0 : -1.0, moveAngle};
    }

    void setBrainFromNetwork(const Network& newBrain) {
        brain = newBrain;
    }

    Network getBrain() const {
        return brain;
    }

    ~EnemyBrain() {
        // Destructor to clean up resources if needed
    }
};

class RangedEnemyBrain {
private:
    Network brain = Network(3, 3, 3, 2); // Inputs: {health/100, distance to player, angle_to_player}, Outputs: {shoot, angle_to_move}
    // daca output[0] > 0, trage in player, altfel, nu trage
    // output[1] este un unghi de miscare, de la 0 la 360 grade
public:
    RangedEnemyBrain() {
        brain.randomize();
    }

    void mutate(double mutationRate) {
        brain.mutate(mutationRate);
    }

    vector<double> result(double health, unsigned enemyX, unsigned enemyY, unsigned playerX, unsigned playerY, double angleToPlayer) {
        vector<double> inputs = {
            health / 100.0, // Normalize health to 0-1
            static_cast<double>(sqrt(pow(enemyX - playerX, 2) + pow(enemyY - playerY, 2))) / 600.0, // Distance to player normalized
            angleToPlayer / 360.0 // Normalize angle to 0-1
        };
        auto output = brain.result(inputs);
        bool shoot = output[0] > 0; // If output[0] > 0, shoot
        double moveAngle = output[1] * 180.0 + 180.0; // Convert to degrees and adjust to 0-360 range
        return {shoot ? 1.0 : 0.0, moveAngle}; // Return 1.0 for shoot, 0.0 for not shoot
    }

    void setBrainFromNetwork(const Network& newBrain) {
        brain = newBrain;
    }

    Network getBrain() const {
        return brain;
    }
};

#endif 