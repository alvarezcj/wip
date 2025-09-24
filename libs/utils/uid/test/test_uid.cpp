#include "uid.h"

#include <gtest/gtest.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <regex>

using namespace wip::utils::uid;

class UIDTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup for each test
    }
    
    void TearDown() override {
        // Cleanup after each test
    }
    
    // Helper function to check if string matches UUID format
    bool is_valid_uuid_format(const std::string& uuid_str) {
        std::regex uuid_regex(
            "^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$",
            std::regex_constants::icase
        );
        return std::regex_match(uuid_str, uuid_regex);
    }
};

// Test default constructor generates valid UUID
TEST_F(UIDTest, DefaultConstructorGeneratesValidUUID) {
    UID uid;
    std::string uuid_str = uid.to_string();
    
    EXPECT_EQ(uuid_str.length(), 36);
    EXPECT_TRUE(is_valid_uuid_format(uuid_str));
    EXPECT_FALSE(uid.is_null());
}

// Test static generate method
TEST_F(UIDTest, StaticGenerateMethod) {
    UID uid = UID::generate();
    std::string uuid_str = uid.to_string();
    
    EXPECT_EQ(uuid_str.length(), 36);
    EXPECT_TRUE(is_valid_uuid_format(uuid_str));
    EXPECT_FALSE(uid.is_null());
}

// Test uniqueness of generated UIDs
TEST_F(UIDTest, UniquenessOfGeneratedUIDs) {
    const int num_uids = 1000;
    std::unordered_set<std::string> uid_strings;
    
    for (int i = 0; i < num_uids; ++i) {
        UID uid = UID::generate();
        std::string uid_str = uid.to_string();
        
        // Check that we haven't seen this UID before
        EXPECT_TRUE(uid_strings.find(uid_str) == uid_strings.end()) 
            << "Duplicate UID found: " << uid_str;
        
        uid_strings.insert(uid_str);
    }
    
    EXPECT_EQ(uid_strings.size(), static_cast<size_t>(num_uids));
}

// Test string constructor and round-trip conversion
TEST_F(UIDTest, StringConstructorAndRoundTrip) {
    // Generate a UID and convert to string
    UID original = UID::generate();
    std::string uuid_str = original.to_string();
    
    // Create new UID from string
    UID from_string(uuid_str);
    
    // Should be equal to original
    EXPECT_EQ(original, from_string);
    EXPECT_EQ(original.to_string(), from_string.to_string());
}

// Test invalid string constructor
TEST_F(UIDTest, InvalidStringConstructor) {
    std::vector<std::string> invalid_uuids = {
        "",
        "invalid",
        "12345678-1234-1234-1234-12345678901",  // Too short
        "12345678-1234-1234-1234-1234567890123", // Too long
        "12345678_1234_1234_1234_123456789012",  // Wrong separators
        "1234567g-1234-1234-1234-123456789012",  // Invalid hex character
        "12345678-1234-1234-1234",               // Missing parts
        "12345678-1234-1234-1234-123456789012-extra", // Extra parts
    };
    
    for (const auto& invalid_uuid : invalid_uuids) {
        EXPECT_THROW({
            UID uid(invalid_uuid);
        }, std::invalid_argument) << "Should throw for: " << invalid_uuid;
    }
}

// Test null UID
TEST_F(UIDTest, NullUID) {
    UID null_uid = UID::null();
    
    EXPECT_TRUE(null_uid.is_null());
    EXPECT_EQ(null_uid.to_string(), "00000000-0000-0000-0000-000000000000");
    
    // Generated UIDs should not be null
    UID generated = UID::generate();
    EXPECT_FALSE(generated.is_null());
}

// Test Rule of 7: Copy constructor
TEST_F(UIDTest, CopyConstructor) {
    UID original = UID::generate();
    UID copied(original);
    
    EXPECT_EQ(original, copied);
    EXPECT_EQ(original.to_string(), copied.to_string());
    
    // They should be separate objects
    EXPECT_NE(&original, &copied);
}

// Test Rule of 7: Copy assignment operator
TEST_F(UIDTest, CopyAssignmentOperator) {
    UID original = UID::generate();
    UID assigned = UID::generate();
    
    // Ensure they're different initially
    EXPECT_NE(original, assigned);
    
    // Assign
    assigned = original;
    
    EXPECT_EQ(original, assigned);
    EXPECT_EQ(original.to_string(), assigned.to_string());
}

// Test Rule of 7: Move constructor
TEST_F(UIDTest, MoveConstructor) {
    UID original = UID::generate();
    std::string original_string = original.to_string();
    
    UID moved(std::move(original));
    
    EXPECT_EQ(moved.to_string(), original_string);
    // Note: original is in valid but unspecified state after move
}

// Test Rule of 7: Move assignment operator
TEST_F(UIDTest, MoveAssignmentOperator) {
    UID original = UID::generate();
    std::string original_string = original.to_string();
    UID assigned = UID::generate();
    
    assigned = std::move(original);
    
    EXPECT_EQ(assigned.to_string(), original_string);
}

// Test self-assignment
TEST_F(UIDTest, SelfAssignment) {
    UID uid = UID::generate();
    std::string original_string = uid.to_string();
    
    // Self-assignment should not change the UID
    uid = uid;
    EXPECT_EQ(uid.to_string(), original_string);
}

// Test comparison operators
TEST_F(UIDTest, ComparisonOperators) {
    UID uid1 = UID::generate();
    UID uid2 = UID::generate();
    UID uid1_copy(uid1);
    
    // Equality
    EXPECT_TRUE(uid1 == uid1_copy);
    EXPECT_FALSE(uid1 == uid2);
    
    // Inequality  
    EXPECT_FALSE(uid1 != uid1_copy);
    EXPECT_TRUE(uid1 != uid2);
}

// Test data access
TEST_F(UIDTest, DataAccess) {
    UID uid = UID::generate();
    const auto& data = uid.data();
    
    EXPECT_EQ(data.size(), 16);
    
    // Check UUID version (should be 4)
    EXPECT_EQ((data[6] & 0xF0) >> 4, 4);
    
    // Check variant bits (should be 10xx in binary, i.e., 0x8 or 0x9 or 0xA or 0xB)
    uint8_t variant = (data[8] & 0xC0) >> 6;
    EXPECT_EQ(variant, 2); // 10 in binary = 2 in decimal
}

// Test hash function
TEST_F(UIDTest, HashFunction) {
    const int num_uids = 100;
    std::unordered_map<UID, int> uid_map;
    
    // Insert UIDs into hash map
    for (int i = 0; i < num_uids; ++i) {
        UID uid = UID::generate();
        uid_map[uid] = i;
    }
    
    EXPECT_EQ(uid_map.size(), static_cast<size_t>(num_uids));
    
    // Test that hash values are reasonably distributed
    std::unordered_set<size_t> hash_values;
    std::hash<UID> hasher;
    
    for (const auto& pair : uid_map) {
        hash_values.insert(hasher(pair.first));
    }
    
    // We should have a good distribution of hash values
    // (allowing some collisions but expecting most to be unique)
    EXPECT_GT(hash_values.size(), static_cast<size_t>(num_uids * 0.8));
}

// Test specific valid UUID strings
TEST_F(UIDTest, SpecificValidUUIDStrings) {
    std::vector<std::string> valid_uuids = {
        "550e8400-e29b-41d4-a716-446655440000",
        "6ba7b810-9dad-11d1-80b4-00c04fd430c8",
        "12345678-1234-4234-a234-123456789012",
        "ffffffff-ffff-4fff-bfff-ffffffffffff",
        "00000000-0000-4000-8000-000000000000"
    };
    
    for (const auto& uuid_str : valid_uuids) {
        EXPECT_NO_THROW({
            UID uid(uuid_str);
            EXPECT_EQ(uid.to_string(), uuid_str);
        }) << "Should work for: " << uuid_str;
    }
}

// Test case insensitivity in string parsing
TEST_F(UIDTest, CaseInsensitiveStringParsing) {
    std::string uppercase = "550E8400-E29B-41D4-A716-446655440000";
    std::string lowercase = "550e8400-e29b-41d4-a716-446655440000";
    
    UID uid_upper(uppercase);
    UID uid_lower(lowercase);
    
    EXPECT_EQ(uid_upper, uid_lower);
    // Note: to_string() always returns lowercase
    EXPECT_EQ(uid_upper.to_string(), lowercase);
    EXPECT_EQ(uid_lower.to_string(), lowercase);
}

// Performance test (basic)
TEST_F(UIDTest, BasicPerformanceTest) {
    const int num_iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<UID> uids;
    uids.reserve(num_iterations);
    
    for (int i = 0; i < num_iterations; ++i) {
        uids.push_back(UID::generate());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should be able to generate 10k UIDs in reasonable time (less than 1 second)
    EXPECT_LT(duration.count(), 1000);
    
    // Verify all UIDs are unique
    std::unordered_set<std::string> unique_uids;
    for (const auto& uid : uids) {
        unique_uids.insert(uid.to_string());
    }
    EXPECT_EQ(unique_uids.size(), static_cast<size_t>(num_iterations));
}