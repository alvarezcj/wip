#include "rng.h"
#include <mutex>
#include <numeric>
#include <stdexcept>
#include <thread>

namespace wip::utils::rng {

// Thread-local storage for distributions
thread_local std::uniform_real_distribution<double> RandomGenerator::uniform_real_(0.0, 1.0);

// ==================== RandomGenerator Implementation ====================

RandomGenerator::RandomGenerator() 
    : gen_(std::chrono::high_resolution_clock::now().time_since_epoch().count()) {
}

RandomGenerator::RandomGenerator(result_type seed) 
    : gen_(seed) {
}

void RandomGenerator::seed(result_type seed) {
    gen_.seed(seed);
}

int RandomGenerator::uniform_int(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen_);
}

unsigned int RandomGenerator::uniform_uint(unsigned int min, unsigned int max) {
    std::uniform_int_distribution<unsigned int> dist(min, max);
    return dist(gen_);
}

int64_t RandomGenerator::uniform_int64(int64_t min, int64_t max) {
    std::uniform_int_distribution<int64_t> dist(min, max);
    return dist(gen_);
}

size_t RandomGenerator::uniform_size_t(size_t min, size_t max) {
    std::uniform_int_distribution<size_t> dist(min, max);
    return dist(gen_);
}

float RandomGenerator::uniform_float(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen_);
}

double RandomGenerator::uniform_double(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(gen_);
}

double RandomGenerator::normal(double mean, double stddev) {
    std::normal_distribution<double> dist(mean, stddev);
    return dist(gen_);
}

double RandomGenerator::exponential(double lambda) {
    std::exponential_distribution<double> dist(lambda);
    return dist(gen_);
}

bool RandomGenerator::boolean(double probability) {
    std::bernoulli_distribution dist(probability);
    return dist(gen_);
}

std::string RandomGenerator::string(size_t length, const std::string& alphabet) {
    if (alphabet.empty()) {
        throw std::invalid_argument("Alphabet cannot be empty");
    }
    
    std::string result;
    result.reserve(length);
    
    std::uniform_int_distribution<size_t> dist(0, alphabet.size() - 1);
    for (size_t i = 0; i < length; ++i) {
        result += alphabet[dist(gen_)];
    }
    
    return result;
}

std::string RandomGenerator::hex_string(size_t length) {
    return string(length, "0123456789ABCDEF");
}

std::string RandomGenerator::alphanumeric(size_t length) {
    return string(length, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
}

std::string RandomGenerator::alphabetic(size_t length) {
    return string(length, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

std::string RandomGenerator::numeric(size_t length) {
    return string(length, "0123456789");
}

std::vector<int> RandomGenerator::integers(size_t count, int min, int max) {
    std::vector<int> result;
    result.reserve(count);
    
    std::uniform_int_distribution<int> dist(min, max);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(dist(gen_));
    }
    
    return result;
}

std::vector<double> RandomGenerator::doubles(size_t count, double min, double max) {
    std::vector<double> result;
    result.reserve(count);
    
    std::uniform_real_distribution<double> dist(min, max);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(dist(gen_));
    }
    
    return result;
}

// ==================== Global Functions ====================

RandomGenerator& global() {
    // Thread-safe singleton
    static thread_local RandomGenerator instance;
    return instance;
}

int randint(int min, int max) {
    return global().uniform_int(min, max);
}

double random_double(double min, double max) {
    return global().uniform_double(min, max);
}

bool randbool(double probability) {
    return global().boolean(probability);
}

void set_global_seed(RandomGenerator::result_type seed) {
    global().seed(seed);
}

// ==================== SecureRandomGenerator Implementation ====================

SecureRandomGenerator::SecureRandomGenerator() {
    // random_device is implementation-defined but typically uses system entropy
}

void SecureRandomGenerator::bytes(void* buffer, size_t size) {
    auto* byte_buffer = static_cast<unsigned char*>(buffer);
    for (size_t i = 0; i < size; ++i) {
        byte_buffer[i] = static_cast<unsigned char>(rd_() & 0xFF);
    }
}

std::string SecureRandomGenerator::string(size_t length, const std::string& alphabet) {
    if (alphabet.empty()) {
        throw std::invalid_argument("Alphabet cannot be empty");
    }
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        auto idx = rd_() % alphabet.size();
        result += alphabet[idx];
    }
    
    return result;
}

int SecureRandomGenerator::uniform_int(int min, int max) {
    if (min > max) {
        throw std::invalid_argument("min cannot be greater than max");
    }
    
    // Use rejection sampling for uniform distribution
    const auto range = static_cast<unsigned int>(max - min + 1);
    const auto limit = UINT32_MAX - (UINT32_MAX % range);
    
    unsigned int value;
    do {
        value = rd_();
    } while (value >= limit);
    
    return static_cast<int>(min + (value % range));
}

std::string SecureRandomGenerator::token(size_t length) {
    return string(length, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_");
}

// ==================== FastRandomGenerator Implementation ====================

FastRandomGenerator::FastRandomGenerator() {
    std::random_device rd;
    state_ = rd();
    // Ensure state is never zero for xorshift
    if (state_ == 0) {
        state_ = 1;
    }
}

FastRandomGenerator::FastRandomGenerator(result_type seed) {
    state_ = seed;
    // Ensure state is never zero for xorshift
    if (state_ == 0) {
        state_ = 1;
    }
}

void FastRandomGenerator::seed(result_type seed) {
    state_ = seed;
    // Ensure state is never zero for xorshift
    if (state_ == 0) {
        state_ = 1;
    }
}

FastRandomGenerator::result_type FastRandomGenerator::next() {
    // xorshift64 algorithm
    state_ ^= state_ << 13;
    state_ ^= state_ >> 7;
    state_ ^= state_ << 17;
    return state_;
}

int FastRandomGenerator::uniform_int(int min, int max) {
    if (min > max) {
        throw std::invalid_argument("min cannot be greater than max");
    }
    
    const auto range = static_cast<uint64_t>(max - min + 1);
    const auto limit = UINT64_MAX - (UINT64_MAX % range);
    
    uint64_t value;
    do {
        value = next();
    } while (value >= limit);
    
    return static_cast<int>(min + (value % range));
}

float FastRandomGenerator::uniform_float() {
    // Use upper 24 bits for float precision
    const uint32_t value = static_cast<uint32_t>(next() >> 32);
    return static_cast<float>(value >> 8) * (1.0f / 16777216.0f); // 2^24
}

double FastRandomGenerator::uniform_double() {
    // Use upper 53 bits for double precision
    const uint64_t value = next() >> 11;
    return static_cast<double>(value) * (1.0 / 9007199254740992.0); // 2^53
}

} // namespace wip::utils::rng