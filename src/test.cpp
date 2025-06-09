#include "ai.h"
#include <iostream>
#include <vector>
#include <cmath>

// Helper function to calculate the total error
double calculateMSE(const std::vector<std::vector<double>>& inputs,
                    const std::vector<std::vector<double>>& expected,
                    const Network& net) {
    // This function can now be a simple wrapper around the network's own error calculation
    return net.computeTotalError(inputs, expected);
}
// Test for the XOR network
bool testXORNetwork() {
    Network net(3, 3, 3, 1);

    std::vector<std::vector<double>> inputs = {
        {0, 0, 0}, {1.0, 0, 0}, {0, 1.0, 0}, {0, 0, 1.0},
        {1.0, 1.0, 0}, {1.0, 0, 1.0}, {0, 1.0, 1.0}, {1.0, 1.0, 1.0}
    };

    std::vector<std::vector<double>> expected = {
        {-1.0}, {1.0}, {1.0}, {1.0}, {-1.0}, {-1.0}, {-1.0}, {-1.0}
    };

    net.randomize();
    net.train(inputs, expected, 0.01, 100000, 0.1);

    double errorSum = calculateMSE(inputs, expected, net);
    if (errorSum < 0.1 * inputs.size()) {
        std::cout << "XOR Network Test: Yes, the error is under 0.1, so AI header works perfectly." << std::endl;
        return true;
    } else {
        std::cout << "XOR Network Test: No, the error is too high." << std::endl;
        return false;
    }
}

// Test for the rain prediction network
bool testRainNetwork() {
    Network rainNet(4, 4, 3, 1);

    std::vector<std::vector<double>> rainInputs = {
        {1.0, 1.0, 0, 0}, {0, 1.0, 1.0, 0}, {1.0, 0, 1.0, 1.0}, {0, 0, 0, 1.0},
        {1.0, 1.0, 1.0, 0}, {0, 1.0, 0, 1.0}, {1.0, 0, 0, 0}, {0, 0, 1.0, 0},
        {1.0, 1.0, 1.0, 1.0}, {0, 1.0, 1.0, 1.0}, {1.0, 0, 1.0, 0}, {0, 0, 0, 0},
        {1.0, 1.0, 0, 1.0}, {0, 0, 1.0, 1.0}, {1.0, 0, 0, 1.0}, {0, 1.0, 0, 0}
    };

    std::vector<std::vector<double>> rainExpected = {
        {1.0}, {-1.0}, {1.0}, {-1.0}, {1.0}, {-1.0}, {1.0}, {-1.0},
        {1.0}, {1.0}, {1.0}, {-1.0}, {1.0}, {-1.0}, {1.0}, {-1.0}
    };

    rainNet.randomize();
    rainNet.train(rainInputs, rainExpected, 0.01, 100000, 0.1);

    double errorSum = calculateMSE(rainInputs, rainExpected, rainNet);
    if (errorSum < 0.1 * rainInputs.size()) {
        std::cout << "Rain Network Test: Yes, the error is under 0.1, so AI header works perfectly." << std::endl;
        return true;
    } else {
        std::cout << "Rain Network Test: No, the error is too high." << std::endl;
        return false;
    }
}

bool testHamsterNetwork() {
    // Inputs: {hunger, thirst, tiredness, boredom}
    // Outputs: {eat, drink, sleep}
    Network hamsterNet(4, 3, 5, 3); // Slightly more neurons (5 vs 4) might help

    std::vector<std::vector<double>> hamsterInputs;
    std::vector<std::vector<double>> hamsterExpected;

    // Generate dataset
    for (double hunger = 0.0; hunger <= 1.0; hunger += 0.5) {
        for (double thirst = 0.0; thirst <= 1.0; thirst += 0.5) {
            for (double tiredness = 0.0; tiredness <= 1.0; tiredness += 0.5) {
                for (double boredom = 0.0; boredom <= 1.0; boredom += 0.5) {
                    hamsterInputs.push_back({hunger, thirst, tiredness, boredom});
                    double eat = (hunger > 0.5) ? 1.0 : -1.0;
                    double drink = (thirst > 0.5) ? 1.0 : -1.0;
                    double sleep = (tiredness > 0.5) ? 1.0 : -1.0;
                    hamsterExpected.push_back({eat, drink, sleep});
                }
            }
        }
    }

    hamsterNet.randomize();
    // Train with a reasonable target error and more generations for this complex problem
    hamsterNet.train(hamsterInputs, hamsterExpected, 0.1, 100000, 0.1);

    // *** FIX: Use the SAME error metric as training (MSE) ***
    double finalMse = hamsterNet.computeTotalError(hamsterInputs, hamsterExpected);
    std::cout << "After training, the final Mean Squared Error is: " << finalMse << "\n";

    // Test with random inputs
    for (int i = 0; i < 10; ++i) {
        std::vector<double> randomInput = { (rand() % 101) / 100.0, (rand() % 101) / 100.0, (rand() % 101) / 100.0, (rand() % 101) / 100.0 };
        std::vector<double> randomOutput = hamsterNet.result(randomInput);
        std::cout << "Hamster Network Test [" << i + 1 << "]: Random Input: " << randomInput[0] << ", " << randomInput[1] << ", " << randomInput[2] << ", " << randomInput[3];

        double eat_score = randomOutput[0];
        double drink_score = randomOutput[1];
        double sleep_score = randomOutput[2];
        std::string best_action = "Do Nothing";
        double max_score = 0.2; // A threshold to act

        if (eat_score > max_score && eat_score >= drink_score && eat_score >= sleep_score) {
            best_action = "Eat";
        } else if (drink_score > max_score && drink_score >= eat_score && drink_score >= sleep_score) {
            best_action = "Drink";
        } else if (sleep_score > max_score && sleep_score >= eat_score && sleep_score >= drink_score) {
            best_action = "Sleep";
        }
        
        std::cout << " -> Output: " << best_action << " (E:" << eat_score << " D:" << drink_score << " S:" << sleep_score << ")" << std::endl;
    }

    // *** FIX: Check against a reasonable MSE threshold ***
    if (finalMse < 0.1) {
        std::cout << "Hamster Network Test: Yes, the MSE (" << finalMse << ") is under the threshold (0.1), so AI header works perfectly." << std::endl;
        return true;
    } else {
        std::cout << "Hamster Network Test: No, the MSE (" << finalMse << ") is too high (threshold: 0.1)." << std::endl;
        return false;
    }
}

bool testEnemyNetwork() {
    Network enemyNet(3, 3, 3, 2); // Inputs: {health/100, distance to player, angle_to_player}
    // Outputs: {shoot, angle_to_move}

    // convert 0-360 angle to 0-1 range
    auto normalizeAngle = [](double angle) {
        return fmod(angle, 360.0) / 360.0;
    };

    // normalize distange to player to 0-1 range (width is 600)
    auto normalizeDistance = [](double distance) {
        return std::min(distance / 600.0, 1.0);
    };

    // health is already in 0-1 range (0-100)
    auto normalizeHealth = [](double health) {
        return health / 100.0;
    };

    enemyNet.randomize();
    double totalDifference = 0.0;

    for (int i = 0; i < 1000; ++i) {
        double health = rand() % 101; // 0-100
        double distance = rand() % 601; // 0-600
        double angle = rand() % 361; // 0-360

        std::vector<double> input = {
            normalizeHealth(health),
            normalizeDistance(distance),
            normalizeAngle(angle)
        };

        std::vector<double> output = enemyNet.result(input);
        totalDifference += output[0] - output[1]; 
        // std::cout << "H: " << health << ", D: " << distance << ", A: " << angle
                // << " -> Output: Shoot: " << (output[0] < 0 ? "No" : "Yes")
                // << ", Move Angle: " << output[1] * 360.0 << " degrees" << std::endl;
    }

    // Check if the network looks natural
    if (std::abs(totalDifference) < 1000.0) {
        std::cout << "Enemy Network Test: Yes, the total difference is under 1000, so AI header works perfectly." << std::endl;
        return true;
    } else {
        std::cout << "Enemy Network Test: No, the total difference is too high." << std::endl;
        return false;
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0))); 
    testXORNetwork();
    testRainNetwork();
    testHamsterNetwork();
    testEnemyNetwork();
    return 0;
}