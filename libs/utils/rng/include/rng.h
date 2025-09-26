#pragma once

#include <random>
#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <memory>
#include <array>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>
#include <numeric>

namespace wip::utils::rng {

/**
 * @brief High-quality random number generator based on Mersenne Twister
 * Thread-safe and provides various distribution utilities
 */
class RandomGenerator {
public:
    using result_type = std::mt19937_64::result_type;
    
    /**
     * @brief Construct with time-based seed
     */
    RandomGenerator();
    
    /**
     * @brief Construct with specific seed
     * @param seed The seed value for reproducible randomness
     */
    explicit RandomGenerator(result_type seed);
    
    /**
     * @brief Seed the generator with a new value
     * @param seed The new seed value
     */
    void seed(result_type seed);
    
    /**
     * @brief Get the underlying generator (for custom distributions)
     * @return Reference to the internal generator
     */
    std::mt19937_64& generator() { return gen_; }
    
    // ==================== Integer Distributions ====================
    
    /**
     * @brief Generate random integer in range [min, max] (inclusive)
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random integer in range
     */
    int uniform_int(int min, int max);
    
    /**
     * @brief Generate random unsigned integer in range [min, max] (inclusive)
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random unsigned integer in range
     */
    unsigned int uniform_uint(unsigned int min, unsigned int max);
    
    /**
     * @brief Generate random 64-bit integer in range [min, max] (inclusive)
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random 64-bit integer in range
     */
    int64_t uniform_int64(int64_t min, int64_t max);
    
    /**
     * @brief Generate random size_t in range [min, max] (inclusive)
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random size_t in range
     */
    size_t uniform_size_t(size_t min, size_t max);
    
    // ==================== Floating Point Distributions ====================
    
    /**
     * @brief Generate random float in range [min, max)
     * @param min Minimum value (inclusive)
     * @param max Maximum value (exclusive)
     * @return Random float in range
     */
    float uniform_float(float min = 0.0f, float max = 1.0f);
    
    /**
     * @brief Generate random double in range [min, max)
     * @param min Minimum value (inclusive)
     * @param max Maximum value (exclusive)
     * @return Random double in range
     */
    double uniform_double(double min = 0.0, double max = 1.0);
    
    /**
     * @brief Generate random number from normal (Gaussian) distribution
     * @param mean The mean of the distribution
     * @param stddev The standard deviation
     * @return Random number from normal distribution
     */
    double normal(double mean = 0.0, double stddev = 1.0);
    
    /**
     * @brief Generate random number from exponential distribution
     * @param lambda The rate parameter (lambda)
     * @return Random number from exponential distribution
     */
    double exponential(double lambda = 1.0);
    
    // ==================== Boolean and Choice ====================
    
    /**
     * @brief Generate random boolean
     * @param probability Probability of returning true (0.0 to 1.0)
     * @return Random boolean
     */
    bool boolean(double probability = 0.5);
    
    /**
     * @brief Choose random element from container
     * @tparam Container Type of container (vector, array, etc.)
     * @param container The container to choose from
     * @return Random element from container
     */
    template<typename Container>
    auto choice(const Container& container) -> decltype(container[0]);
    
    /**
     * @brief Choose random element from initializer list
     * @tparam T Element type
     * @param elements The elements to choose from
     * @return Random element
     */
    template<typename T>
    T choice(std::initializer_list<T> elements);
    
    /**
     * @brief Random sample from container without replacement
     * @param container The container to sample from
     * @param count Number of elements to sample
     * @return Vector containing sampled elements
     */
    template<typename Container>
    auto sample(const Container& container, size_t count) -> std::vector<typename Container::value_type>;
    
    // ==================== String Generation ====================
    
    /**
     * @brief Generate random string from alphabet
     * @param length Length of the string to generate
     * @param alphabet Character set to choose from
     * @return Random string
     */
    std::string string(size_t length, const std::string& alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    
    /**
     * @brief Generate random hexadecimal string
     * @param length Length of the hex string
     * @return Random hex string
     */
    std::string hex_string(size_t length);
    
    /**
     * @brief Generate random alphanumeric string
     * @param length Length of the string
     * @return Random alphanumeric string
     */
    std::string alphanumeric(size_t length);
    
    /**
     * @brief Generate random alphabetic string (letters only)
     * @param length Length of the string
     * @return Random alphabetic string
     */
    std::string alphabetic(size_t length);
    
    /**
     * @brief Generate random numeric string
     * @param length Length of the string
     * @return Random numeric string
     */
    std::string numeric(size_t length);
    
    // ==================== Collection Utilities ====================
    
    /**
     * @brief Shuffle elements in container randomly
     * @tparam Container Type of container
     * @param container The container to shuffle (modified in-place)
     */
    template<typename Container>
    void shuffle(Container& container);
    
    /**
     * @brief Generate vector of random integers
     * @param count Number of integers to generate
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Vector of random integers
     */
    std::vector<int> integers(size_t count, int min, int max);
    
    /**
     * @brief Generate vector of random doubles
     * @param count Number of doubles to generate
     * @param min Minimum value (inclusive)
     * @param max Maximum value (exclusive)
     * @return Vector of random doubles
     */
    std::vector<double> doubles(size_t count, double min = 0.0, double max = 1.0);
    
    // ==================== Weighted Random ====================
    
    /**
     * @brief Choose element with weighted probabilities
     * @tparam T Element type
     * @param elements Vector of elements to choose from
     * @param weights Vector of weights (must be same size as elements)
     * @return Weighted random choice
     */
    template<typename T>
    T weighted_choice(const std::vector<T>& elements, const std::vector<double>& weights);
    
private:
    std::mt19937_64 gen_;
    
    // Thread-local distributions for performance
    thread_local static std::uniform_real_distribution<double> uniform_real_;
};

// ==================== Global Utilities ====================

/**
 * @brief Global thread-safe random generator instance
 * Automatically seeded on first use
 */
RandomGenerator& global();

/**
 * @brief Convenience function for random integer
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return Random integer in range
 */
int randint(int min, int max);

/**
 * @brief Convenience function for random double
 * @param min Minimum value (inclusive)
 * @param max Maximum value (exclusive)
 * @return Random double in range
 */
double random_double(double min = 0.0, double max = 1.0);

/**
 * @brief Convenience function for random boolean
 * @param probability Probability of returning true
 * @return Random boolean
 */
bool randbool(double probability = 0.5);

/**
 * @brief Convenience function for random choice
 * @tparam T Element type
 * @param elements Initializer list of elements
 * @return Random element
 */
template<typename T>
T choice(std::initializer_list<T> elements);

/**
 * @brief Set global seed for reproducible randomness
 * @param seed The seed value
 */
void set_global_seed(RandomGenerator::result_type seed);

// ==================== Specialized Generators ====================

/**
 * @brief Cryptographically secure random number generator
 * Uses system entropy source, slower but more secure
 */
class SecureRandomGenerator {
public:
    SecureRandomGenerator();
    
    /**
     * @brief Generate cryptographically secure random bytes
     * @param buffer Output buffer
     * @param size Number of bytes to generate
     */
    void bytes(void* buffer, size_t size);
    
    /**
     * @brief Generate secure random string
     * @param length Length of string
     * @param alphabet Character set to use
     * @return Secure random string
     */
    std::string string(size_t length, const std::string& alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    
    /**
     * @brief Generate secure random integer
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Secure random integer
     */
    int uniform_int(int min, int max);
    
    /**
     * @brief Generate secure random token/password
     * @param length Length of token
     * @return Secure random token
     */
    std::string token(size_t length = 32);
    
private:
    std::random_device rd_;
};

/**
 * @brief Fast, lightweight random number generator
 * Based on xorshift algorithm, very fast but lower quality
 * Good for simulations where speed matters more than statistical quality
 */
class FastRandomGenerator {
public:
    using result_type = uint64_t;
    
    FastRandomGenerator();
    explicit FastRandomGenerator(result_type seed);
    
    void seed(result_type seed);
    
    /**
     * @brief Generate next random number
     * @return Random 64-bit unsigned integer
     */
    result_type next();
    
    /**
     * @brief Generate random integer in range [min, max]
     * @param min Minimum value
     * @param max Maximum value
     * @return Random integer in range
     */
    int uniform_int(int min, int max);
    
    /**
     * @brief Generate random float in range [0, 1)
     * @return Random float
     */
    float uniform_float();
    
    /**
     * @brief Generate random double in range [0, 1)
     * @return Random double
     */
    double uniform_double();
    
private:
    uint64_t state_;
};

// Template implementations
template<typename Container>
auto RandomGenerator::choice(const Container& container) -> decltype(container[0]) {
    if (container.empty()) {
        throw std::invalid_argument("Cannot choose from empty container");
    }
    auto idx = uniform_size_t(0, container.size() - 1);
    return container[idx];
}

template<typename T>
T RandomGenerator::choice(std::initializer_list<T> elements) {
    if (elements.size() == 0) {
        throw std::invalid_argument("Cannot choose from empty list");
    }
    auto idx = uniform_size_t(0, elements.size() - 1);
    return *(elements.begin() + idx);
}

template<typename Container>
auto RandomGenerator::sample(const Container& container, size_t count) -> std::vector<typename Container::value_type> {
    if (count > container.size()) {
        throw std::invalid_argument("Sample size cannot be larger than container size");
    }
    
    std::vector<size_t> indices(container.size());
    std::iota(indices.begin(), indices.end(), 0);
    shuffle(indices);
    
    std::vector<typename Container::value_type> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(container[indices[i]]);
    }
    
    return result;
}

template<typename Container>
void RandomGenerator::shuffle(Container& container) {
    std::shuffle(container.begin(), container.end(), gen_);
}

template<typename T>
T RandomGenerator::weighted_choice(const std::vector<T>& elements, const std::vector<double>& weights) {
    if (elements.size() != weights.size()) {
        throw std::invalid_argument("Elements and weights vectors must have same size");
    }
    if (elements.empty()) {
        throw std::invalid_argument("Cannot choose from empty container");
    }
    
    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    auto idx = dist(gen_);
    return elements[idx];
}

template<typename T>
T choice(std::initializer_list<T> elements) {
    return global().choice(elements);
}

} // namespace wip::utils::rng