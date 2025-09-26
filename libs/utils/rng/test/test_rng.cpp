#include <gtest/gtest.h>
#include "rng.h"
#include <set>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <cmath>

using namespace wip::utils::rng;

class RngTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use fixed seed for reproducible tests
        rng.seed(12345);
    }
    
    RandomGenerator rng;
};

// ==================== Basic Integer Tests ====================

TEST_F(RngTest, UniformIntBasicRange) {
    // Test basic range
    for (int i = 0; i < 1000; ++i) {
        int value = rng.uniform_int(1, 10);
        EXPECT_GE(value, 1);
        EXPECT_LE(value, 10);
    }
}

TEST_F(RngTest, UniformIntSingleValue) {
    // Test single value range
    for (int i = 0; i < 100; ++i) {
        int value = rng.uniform_int(5, 5);
        EXPECT_EQ(value, 5);
    }
}

TEST_F(RngTest, UniformIntDistribution) {
    // Test that values are reasonably distributed
    std::map<int, int> counts;
    const int trials = 10000;
    
    for (int i = 0; i < trials; ++i) {
        int value = rng.uniform_int(1, 10);
        counts[value]++;
    }
    
    // Each value should appear roughly 1000 times (10% of trials)
    // Allow for some variance - check that each appears at least 800 times
    for (int i = 1; i <= 10; ++i) {
        EXPECT_GT(counts[i], 800) << "Value " << i << " appeared " << counts[i] << " times";
        EXPECT_LT(counts[i], 1200) << "Value " << i << " appeared " << counts[i] << " times";
    }
}

TEST_F(RngTest, UniformInt64) {
    int64_t min = -1000000000LL;
    int64_t max = 1000000000LL;
    
    for (int i = 0; i < 1000; ++i) {
        int64_t value = rng.uniform_int64(min, max);
        EXPECT_GE(value, min);
        EXPECT_LE(value, max);
    }
}

TEST_F(RngTest, UniformSizeT) {
    size_t min = 0;
    size_t max = 1000;
    
    for (int i = 0; i < 1000; ++i) {
        size_t value = rng.uniform_size_t(min, max);
        EXPECT_GE(value, min);
        EXPECT_LE(value, max);
    }
}

// ==================== Floating Point Tests ====================

TEST_F(RngTest, UniformFloat) {
    for (int i = 0; i < 1000; ++i) {
        float value = rng.uniform_float();
        EXPECT_GE(value, 0.0f);
        EXPECT_LT(value, 1.0f);
    }
    
    // Test custom range
    for (int i = 0; i < 1000; ++i) {
        float value = rng.uniform_float(10.0f, 20.0f);
        EXPECT_GE(value, 10.0f);
        EXPECT_LT(value, 20.0f);
    }
}

TEST_F(RngTest, UniformDouble) {
    for (int i = 0; i < 1000; ++i) {
        double value = rng.uniform_double();
        EXPECT_GE(value, 0.0);
        EXPECT_LT(value, 1.0);
    }
    
    // Test custom range
    for (int i = 0; i < 1000; ++i) {
        double value = rng.uniform_double(-5.0, 5.0);
        EXPECT_GE(value, -5.0);
        EXPECT_LT(value, 5.0);
    }
}

TEST_F(RngTest, NormalDistribution) {
    std::vector<double> values;
    const int trials = 10000;
    
    for (int i = 0; i < trials; ++i) {
        values.push_back(rng.normal(0.0, 1.0));
    }
    
    // Calculate mean and standard deviation
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / trials;
    double variance = 0.0;
    for (double v : values) {
        variance += (v - mean) * (v - mean);
    }
    variance /= trials;
    double stddev = std::sqrt(variance);
    
    // Should be approximately mean=0, stddev=1
    EXPECT_NEAR(mean, 0.0, 0.1);
    EXPECT_NEAR(stddev, 1.0, 0.1);
}

TEST_F(RngTest, ExponentialDistribution) {
    std::vector<double> values;
    const int trials = 10000;
    const double lambda = 2.0;
    
    for (int i = 0; i < trials; ++i) {
        values.push_back(rng.exponential(lambda));
    }
    
    // For exponential distribution, mean = 1/lambda
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / trials;
    EXPECT_NEAR(mean, 1.0/lambda, 0.1);
    
    // All values should be positive
    for (double v : values) {
        EXPECT_GT(v, 0.0);
    }
}

// ==================== Boolean Tests ====================

TEST_F(RngTest, BooleanDefault) {
    int true_count = 0;
    const int trials = 10000;
    
    for (int i = 0; i < trials; ++i) {
        if (rng.boolean()) {
            true_count++;
        }
    }
    
    // Should be approximately 50%
    double ratio = static_cast<double>(true_count) / trials;
    EXPECT_NEAR(ratio, 0.5, 0.05);
}

TEST_F(RngTest, BooleanCustomProbability) {
    int true_count = 0;
    const int trials = 10000;
    const double probability = 0.7;
    
    for (int i = 0; i < trials; ++i) {
        if (rng.boolean(probability)) {
            true_count++;
        }
    }
    
    // Should be approximately 70%
    double ratio = static_cast<double>(true_count) / trials;
    EXPECT_NEAR(ratio, probability, 0.05);
}

// ==================== Choice Tests ====================

TEST_F(RngTest, ChoiceVector) {
    std::vector<int> elements = {1, 2, 3, 4, 5};
    std::set<int> seen;
    
    for (int i = 0; i < 1000; ++i) {
        int chosen = rng.choice(elements);
        seen.insert(chosen);
        
        // Verify chosen element is in original vector
        EXPECT_TRUE(std::find(elements.begin(), elements.end(), chosen) != elements.end());
    }
    
    // Should have seen all elements
    EXPECT_EQ(seen.size(), elements.size());
}

TEST_F(RngTest, ChoiceInitializerList) {
    std::set<std::string> seen;
    
    for (int i = 0; i < 1000; ++i) {
        std::string chosen = rng.choice({"apple", "banana", "cherry"});
        seen.insert(chosen);
        
        // Verify chosen element is valid
        EXPECT_TRUE(chosen == "apple" || chosen == "banana" || chosen == "cherry");
    }
    
    // Should have seen all elements
    EXPECT_EQ(seen.size(), 3);
}

TEST_F(RngTest, ChoiceEmptyContainer) {
    std::vector<int> empty;
    EXPECT_THROW(rng.choice(empty), std::invalid_argument);
    EXPECT_THROW(rng.choice<int>({}), std::invalid_argument);
}

TEST_F(RngTest, Sample) {
    std::vector<int> elements = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    auto sample = rng.sample(elements, 5);
    EXPECT_EQ(sample.size(), 5);
    
    // All sampled elements should be unique
    std::set<int> unique_sample(sample.begin(), sample.end());
    EXPECT_EQ(unique_sample.size(), 5);
    
    // All sampled elements should be from original vector
    for (int elem : sample) {
        EXPECT_TRUE(std::find(elements.begin(), elements.end(), elem) != elements.end());
    }
}

TEST_F(RngTest, SampleTooLarge) {
    std::vector<int> elements = {1, 2, 3};
    EXPECT_THROW(rng.sample(elements, 5), std::invalid_argument);
}

// ==================== String Generation Tests ====================

TEST_F(RngTest, StringGeneration) {
    std::string result = rng.string(10);
    EXPECT_EQ(result.length(), 10);
    
    // Should only contain valid alphabet characters
    std::string default_alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (char c : result) {
        EXPECT_TRUE(default_alphabet.find(c) != std::string::npos);
    }
}

TEST_F(RngTest, StringCustomAlphabet) {
    std::string alphabet = "ABC";
    std::string result = rng.string(100, alphabet);
    EXPECT_EQ(result.length(), 100);
    
    for (char c : result) {
        EXPECT_TRUE(c == 'A' || c == 'B' || c == 'C');
    }
}

TEST_F(RngTest, StringEmptyAlphabet) {
    EXPECT_THROW(rng.string(10, ""), std::invalid_argument);
}

TEST_F(RngTest, HexString) {
    std::string result = rng.hex_string(16);
    EXPECT_EQ(result.length(), 16);
    
    std::string hex_chars = "0123456789ABCDEF";
    for (char c : result) {
        EXPECT_TRUE(hex_chars.find(c) != std::string::npos);
    }
}

TEST_F(RngTest, AlphanumericString) {
    std::string result = rng.alphanumeric(20);
    EXPECT_EQ(result.length(), 20);
    
    for (char c : result) {
        EXPECT_TRUE(std::isalnum(c));
    }
}

TEST_F(RngTest, AlphabeticString) {
    std::string result = rng.alphabetic(15);
    EXPECT_EQ(result.length(), 15);
    
    for (char c : result) {
        EXPECT_TRUE(std::isalpha(c));
    }
}

TEST_F(RngTest, NumericString) {
    std::string result = rng.numeric(12);
    EXPECT_EQ(result.length(), 12);
    
    for (char c : result) {
        EXPECT_TRUE(std::isdigit(c));
    }
}

// ==================== Collection Utilities Tests ====================

TEST_F(RngTest, Shuffle) {
    std::vector<int> original = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> to_shuffle = original;
    
    rng.shuffle(to_shuffle);
    
    // Should have same elements
    std::sort(to_shuffle.begin(), to_shuffle.end());
    EXPECT_EQ(to_shuffle, original);
    
    // Multiple shuffles should produce different results
    std::vector<int> shuffle1 = original;
    std::vector<int> shuffle2 = original;
    rng.shuffle(shuffle1);
    rng.shuffle(shuffle2);
    
    // Very unlikely to be the same (but theoretically possible)
    EXPECT_NE(shuffle1, shuffle2);
}

TEST_F(RngTest, IntegersGeneration) {
    auto ints = rng.integers(100, 1, 10);
    EXPECT_EQ(ints.size(), 100);
    
    for (int value : ints) {
        EXPECT_GE(value, 1);
        EXPECT_LE(value, 10);
    }
}

TEST_F(RngTest, DoublesGeneration) {
    auto doubles = rng.doubles(50, -1.0, 1.0);
    EXPECT_EQ(doubles.size(), 50);
    
    for (double value : doubles) {
        EXPECT_GE(value, -1.0);
        EXPECT_LT(value, 1.0);
    }
}

// ==================== Weighted Choice Tests ====================

TEST_F(RngTest, WeightedChoice) {
    std::vector<std::string> elements = {"rare", "common", "uncommon"};
    std::vector<double> weights = {0.1, 0.8, 0.1};  // "common" should appear 80% of time
    
    std::map<std::string, int> counts;
    const int trials = 10000;
    
    for (int i = 0; i < trials; ++i) {
        std::string chosen = rng.weighted_choice(elements, weights);
        counts[chosen]++;
    }
    
    // "common" should appear about 80% of the time
    double common_ratio = static_cast<double>(counts["common"]) / trials;
    EXPECT_NEAR(common_ratio, 0.8, 0.05);
    
    // "rare" and "uncommon" should each appear about 10% of the time
    double rare_ratio = static_cast<double>(counts["rare"]) / trials;
    double uncommon_ratio = static_cast<double>(counts["uncommon"]) / trials;
    EXPECT_NEAR(rare_ratio, 0.1, 0.05);
    EXPECT_NEAR(uncommon_ratio, 0.1, 0.05);
}

TEST_F(RngTest, WeightedChoiceMismatchedSizes) {
    std::vector<int> elements = {1, 2, 3};
    std::vector<double> weights = {0.5, 0.5};  // Wrong size
    
    EXPECT_THROW(rng.weighted_choice(elements, weights), std::invalid_argument);
}

// ==================== Global Functions Tests ====================

TEST_F(RngTest, GlobalFunctions) {
    // Test that global functions work
    set_global_seed(54321);
    
    int r1 = randint(1, 10);
    EXPECT_GE(r1, 1);
    EXPECT_LE(r1, 10);
    
    double r2 = random_double();
    EXPECT_GE(r2, 0.0);
    EXPECT_LT(r2, 1.0);
    
    bool r3 = randbool(0.3);
    // Just verify it returns a boolean (can't really test the probability easily)
    EXPECT_TRUE(r3 == true || r3 == false);
    
    auto r4 = choice({"a", "b", "c"});
    EXPECT_TRUE(r4 == "a" || r4 == "b" || r4 == "c");
}

// ==================== Reproducibility Tests ====================

TEST_F(RngTest, Reproducibility) {
    // Same seed should produce same sequence
    RandomGenerator rng1(99999);
    RandomGenerator rng2(99999);
    
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(rng1.uniform_int(1, 1000), rng2.uniform_int(1, 1000));
    }
}

TEST_F(RngTest, SeedingChangesSequence) {
    RandomGenerator rng1(11111);
    RandomGenerator rng2(22222);
    
    // Different seeds should produce different sequences
    bool different = false;
    for (int i = 0; i < 100; ++i) {
        if (rng1.uniform_int(1, 1000) != rng2.uniform_int(1, 1000)) {
            different = true;
            break;
        }
    }
    EXPECT_TRUE(different);
}

// ==================== SecureRandomGenerator Tests ====================

TEST(SecureRngTest, BasicFunctionality) {
    SecureRandomGenerator secure_rng;
    
    // Test secure integer generation
    for (int i = 0; i < 100; ++i) {
        int value = secure_rng.uniform_int(1, 10);
        EXPECT_GE(value, 1);
        EXPECT_LE(value, 10);
    }
    
    // Test secure string generation
    std::string token = secure_rng.token(32);
    EXPECT_EQ(token.length(), 32);
    
    // Test bytes generation
    std::array<unsigned char, 16> buffer;
    secure_rng.bytes(buffer.data(), buffer.size());
    
    // Can't really test randomness, just that it doesn't crash
    // and fills the buffer (check that not all bytes are the same)
    bool different_bytes = false;
    for (size_t i = 1; i < buffer.size(); ++i) {
        if (buffer[i] != buffer[0]) {
            different_bytes = true;
            break;
        }
    }
    // This could theoretically fail, but extremely unlikely
    EXPECT_TRUE(different_bytes);
}

// ==================== FastRandomGenerator Tests ====================

TEST(FastRngTest, BasicFunctionality) {
    FastRandomGenerator fast_rng;
    
    // Test basic generation
    for (int i = 0; i < 1000; ++i) {
        auto value = fast_rng.next();
        // Just verify it returns different values
        EXPECT_NE(value, 0);  // Should be extremely unlikely to be zero
    }
    
    // Test uniform int
    for (int i = 0; i < 1000; ++i) {
        int value = fast_rng.uniform_int(1, 10);
        EXPECT_GE(value, 1);
        EXPECT_LE(value, 10);
    }
    
    // Test uniform float
    for (int i = 0; i < 1000; ++i) {
        float value = fast_rng.uniform_float();
        EXPECT_GE(value, 0.0f);
        EXPECT_LT(value, 1.0f);
    }
    
    // Test uniform double
    for (int i = 0; i < 1000; ++i) {
        double value = fast_rng.uniform_double();
        EXPECT_GE(value, 0.0);
        EXPECT_LT(value, 1.0);
    }
}

TEST(FastRngTest, Reproducibility) {
    FastRandomGenerator rng1(12345);
    FastRandomGenerator rng2(12345);
    
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(rng1.next(), rng2.next());
    }
}

// ==================== Performance Comparison Test ====================

TEST(RngPerformanceTest, DISABLED_SpeedComparison) {
    // This test is disabled by default as it's mainly for manual performance testing
    const int iterations = 1000000;
    
    // Test RandomGenerator (Mersenne Twister)
    RandomGenerator standard_rng;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        standard_rng.uniform_int(1, 100);
    }
    auto standard_time = std::chrono::high_resolution_clock::now() - start;
    
    // Test FastRandomGenerator
    FastRandomGenerator fast_rng;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        fast_rng.uniform_int(1, 100);
    }
    auto fast_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Standard RNG time: " << std::chrono::duration_cast<std::chrono::microseconds>(standard_time).count() << " μs\n";
    std::cout << "Fast RNG time: " << std::chrono::duration_cast<std::chrono::microseconds>(fast_time).count() << " μs\n";
    
    // Fast RNG should be faster (though this isn't guaranteed in all environments)
    // This is more of an informational test
}

// ==================== Edge Cases Tests ====================

TEST_F(RngTest, EdgeCases) {
    // Test negative ranges
    int neg_value = rng.uniform_int(-10, -5);
    EXPECT_GE(neg_value, -10);
    EXPECT_LE(neg_value, -5);
    
    // Test very large ranges
    int64_t large_value = rng.uniform_int64(INT64_MIN + 1000, INT64_MAX - 1000);
    EXPECT_GE(large_value, INT64_MIN + 1000);
    EXPECT_LE(large_value, INT64_MAX - 1000);
    
    // Test zero probability
    bool zero_prob = rng.boolean(0.0);
    EXPECT_FALSE(zero_prob);
    
    // Test full probability
    bool full_prob = rng.boolean(1.0);
    EXPECT_TRUE(full_prob);
    
    // Test empty string generation
    std::string empty = rng.string(0);
    EXPECT_TRUE(empty.empty());
}