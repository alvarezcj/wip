#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include "rng.h"

using namespace wip::utils::rng;

void demonstrate_basic_generation() {
    std::cout << "\n=== Basic Random Generation ===" << std::endl;
    
    RandomGenerator rng;
    
    // Integer generation
    std::cout << "Random integers (1-10): ";
    for (int i = 0; i < 5; ++i) {
        std::cout << rng.uniform_int(1, 10) << " ";
    }
    std::cout << std::endl;
    
    // Floating point generation
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Random doubles (0-1): ";
    for (int i = 0; i < 5; ++i) {
        std::cout << rng.uniform_double() << " ";
    }
    std::cout << std::endl;
    
    // Boolean generation
    std::cout << "Random booleans (70% true): ";
    for (int i = 0; i < 10; ++i) {
        std::cout << (rng.boolean(0.7) ? "T" : "F") << " ";
    }
    std::cout << std::endl;
}

void demonstrate_distributions() {
    std::cout << "\n=== Statistical Distributions ===" << std::endl;
    
    RandomGenerator rng;
    
    // Normal distribution
    std::cout << "Normal distribution (mean=0, stddev=1): ";
    for (int i = 0; i < 5; ++i) {
        std::cout << std::fixed << std::setprecision(2) << rng.normal(0.0, 1.0) << " ";
    }
    std::cout << std::endl;
    
    // Exponential distribution
    std::cout << "Exponential distribution (lambda=2): ";
    for (int i = 0; i < 5; ++i) {
        std::cout << std::fixed << std::setprecision(2) << rng.exponential(2.0) << " ";
    }
    std::cout << std::endl;
}

void demonstrate_string_generation() {
    std::cout << "\n=== String Generation ===" << std::endl;
    
    RandomGenerator rng;
    
    std::cout << "Random alphanumeric (10 chars): " << rng.alphanumeric(10) << std::endl;
    std::cout << "Random hex string (16 chars): " << rng.hex_string(16) << std::endl;
    std::cout << "Random alphabetic (8 chars): " << rng.alphabetic(8) << std::endl;
    std::cout << "Random numeric (6 chars): " << rng.numeric(6) << std::endl;
    std::cout << "Custom alphabet (vowels): " << rng.string(10, "aeiou") << std::endl;
}

void demonstrate_collections() {
    std::cout << "\n=== Collection Operations ===" << std::endl;
    
    RandomGenerator rng;
    
    // Choice from container
    std::vector<std::string> colors = {"red", "blue", "green", "yellow", "purple"};
    std::cout << "Random color: " << rng.choice(colors) << std::endl;
    std::cout << "Random choice from list: " << rng.choice({"apple", "banana", "orange"}) << std::endl;
    
    // Sampling
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto sample = rng.sample(numbers, 3);
    std::cout << "Sample of 3 numbers: ";
    for (int n : sample) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // Shuffling
    std::vector<std::string> deck = {"A", "K", "Q", "J", "10", "9", "8", "7"};
    rng.shuffle(deck);
    std::cout << "Shuffled deck: ";
    for (const std::string& card : deck) {
        std::cout << card << " ";
    }
    std::cout << std::endl;
    
    // Weighted choice
    std::vector<std::string> items = {"common", "rare", "legendary"};
    std::vector<double> weights = {0.7, 0.25, 0.05}; // 70%, 25%, 5%
    std::cout << "Weighted items (10 draws): ";
    for (int i = 0; i < 10; ++i) {
        std::cout << rng.weighted_choice(items, weights).substr(0, 1) << " ";
    }
    std::cout << std::endl;
}

void demonstrate_generators() {
    std::cout << "\n=== Different Generator Types ===" << std::endl;
    
    // Standard (high quality)
    RandomGenerator standard_rng(42);
    std::cout << "Standard RNG (seed 42): ";
    for (int i = 0; i < 5; ++i) {
        std::cout << standard_rng.uniform_int(1, 100) << " ";
    }
    std::cout << std::endl;
    
    // Fast RNG
    FastRandomGenerator fast_rng(42);
    std::cout << "Fast RNG (seed 42): ";
    for (int i = 0; i < 5; ++i) {
        std::cout << fast_rng.uniform_int(1, 100) << " ";
    }
    std::cout << std::endl;
    
    // Secure RNG
    SecureRandomGenerator secure_rng;
    std::cout << "Secure RNG token (32 chars): " << secure_rng.token(32) << std::endl;
}

void demonstrate_global_functions() {
    std::cout << "\n=== Global Convenience Functions ===" << std::endl;
    
    set_global_seed(123);
    
    std::cout << "Global randint(1-6): ";
    for (int i = 0; i < 10; ++i) {
        std::cout << randint(1, 6) << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Global random_double(): ";
    for (int i = 0; i < 5; ++i) {
        std::cout << std::fixed << std::setprecision(3) << random_double() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Global choice: " << choice({"heads", "tails"}) << std::endl;
}

void demonstrate_reproducibility() {
    std::cout << "\n=== Reproducibility ===" << std::endl;
    
    // Same seed should produce same sequence
    RandomGenerator rng1(999);
    RandomGenerator rng2(999);
    
    std::cout << "RNG1 (seed 999): ";
    for (int i = 0; i < 5; ++i) {
        std::cout << rng1.uniform_int(1, 10) << " ";
    }
    std::cout << std::endl;
    
    std::cout << "RNG2 (seed 999): ";
    for (int i = 0; i < 5; ++i) {
        std::cout << rng2.uniform_int(1, 10) << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "🎲 WIP Random Number Generation Library Demo 🎲" << std::endl;
    std::cout << "================================================" << std::endl;
    
    try {
        demonstrate_basic_generation();
        demonstrate_distributions();
        demonstrate_string_generation();
        demonstrate_collections();
        demonstrate_generators();
        demonstrate_global_functions();
        demonstrate_reproducibility();
        
        std::cout << "\n✅ All RNG demonstrations completed successfully!" << std::endl;
        std::cout << "\nThe RNG library provides:" << std::endl;
        std::cout << "• High-quality Mersenne Twister generator" << std::endl;
        std::cout << "• Fast xorshift generator for performance" << std::endl;
        std::cout << "• Cryptographically secure generator" << std::endl;
        std::cout << "• Multiple statistical distributions" << std::endl;
        std::cout << "• String generation utilities" << std::endl;
        std::cout << "• Collection manipulation functions" << std::endl;
        std::cout << "• Global convenience functions" << std::endl;
        std::cout << "• Reproducible seeded generation" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}