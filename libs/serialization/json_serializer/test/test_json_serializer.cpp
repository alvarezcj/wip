#include "json_serializer.h"
#include <gtest/gtest.h>
#include <unordered_map>
#include <map>
#include <vector>

using namespace wip::serialization;

// Test string serialization
TEST(JsonSerializerTest, StringSerialization) {
    JsonSerializer<std::string> serializer;
    std::string test_data = "Hello, World!";
    
    auto json_result = serializer.serialize(test_data);
    EXPECT_TRUE(json_result.is_string());
    EXPECT_EQ(json_result.get<std::string>(), test_data);
    
    auto deserialized = serializer.deserialize(json_result);
    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), test_data);
}

// Test integer serialization
TEST(JsonSerializerTest, IntSerialization) {
    JsonSerializer<int> serializer;
    int test_data = 42;
    
    auto json_result = serializer.serialize(test_data);
    EXPECT_TRUE(json_result.is_number_integer());
    EXPECT_EQ(json_result.get<int>(), test_data);
    
    auto deserialized = serializer.deserialize(json_result);
    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), test_data);
}

// Test double serialization
TEST(JsonSerializerTest, DoubleSerialization) {
    JsonSerializer<double> serializer;
    double test_data = 3.14159;
    
    auto json_result = serializer.serialize(test_data);
    EXPECT_TRUE(json_result.is_number_float());
    EXPECT_DOUBLE_EQ(json_result.get<double>(), test_data);
    
    auto deserialized = serializer.deserialize(json_result);
    ASSERT_TRUE(deserialized.has_value());
    EXPECT_DOUBLE_EQ(deserialized.value(), test_data);
}

// Test boolean serialization
TEST(JsonSerializerTest, BoolSerialization) {
    JsonSerializer<bool> serializer;
    bool test_data = true;
    
    auto json_result = serializer.serialize(test_data);
    EXPECT_TRUE(json_result.is_boolean());
    EXPECT_EQ(json_result.get<bool>(), test_data);
    
    auto deserialized = serializer.deserialize(json_result);
    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), test_data);
}

// Test deserialization failures
TEST(JsonSerializerTest, DeserializationFailures) {
    JsonSerializer<std::string> string_serializer;
    JsonSerializer<int> int_serializer;
    JsonSerializer<double> double_serializer;
    JsonSerializer<bool> bool_serializer;
    
    nlohmann::json wrong_type = 42;
    
    EXPECT_FALSE(string_serializer.deserialize(wrong_type).has_value());
    EXPECT_FALSE(bool_serializer.deserialize(wrong_type).has_value());
    
    nlohmann::json string_type = "not a number";
    EXPECT_FALSE(int_serializer.deserialize(string_type).has_value());
    EXPECT_FALSE(double_serializer.deserialize(string_type).has_value());
}

// Tests for std::vector<T> specialization
TEST(JsonSerializerContainerTest, VectorIntSerializeDeserialize) {
    JsonSerializer<std::vector<int>> serializer;
    std::vector<int> original = {1, 2, 3, 4, 5};
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_array());
    EXPECT_EQ(serialized.size(), 5);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, VectorStringSerializeDeserialize) {
    JsonSerializer<std::vector<std::string>> serializer;
    std::vector<std::string> original = {"hello", "world", "test"};
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_array());
    EXPECT_EQ(serialized.size(), 3);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, VectorEmptySerializeDeserialize) {
    JsonSerializer<std::vector<int>> serializer;
    std::vector<int> original;
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_array());
    EXPECT_EQ(serialized.size(), 0);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, VectorDeserializeInvalidJson) {
    JsonSerializer<std::vector<int>> serializer;
    nlohmann::json invalid_json = "not_an_array";
    
    auto deserialized = serializer.deserialize(invalid_json);
    EXPECT_FALSE(deserialized.has_value());
}

// Tests for std::unordered_map<std::string, T> specialization
TEST(JsonSerializerContainerTest, UnorderedMapIntSerializeDeserialize) {
    JsonSerializer<std::unordered_map<std::string, int>> serializer;
    std::unordered_map<std::string, int> original = {{"key1", 1}, {"key2", 2}, {"key3", 3}};
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_object());
    EXPECT_EQ(serialized.size(), 3);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, UnorderedMapStringSerializeDeserialize) {
    JsonSerializer<std::unordered_map<std::string, std::string>> serializer;
    std::unordered_map<std::string, std::string> original = {{"name", "John"}, {"city", "NYC"}};
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_object());
    EXPECT_EQ(serialized.size(), 2);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, UnorderedMapEmptySerializeDeserialize) {
    JsonSerializer<std::unordered_map<std::string, int>> serializer;
    std::unordered_map<std::string, int> original;
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_object());
    EXPECT_EQ(serialized.size(), 0);
    
    auto deserialized = serializer.deserialize(serialized);
    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, UnorderedMapDeserializeInvalidJson) {
    JsonSerializer<std::unordered_map<std::string, int>> serializer;
    nlohmann::json invalid_json = "not_an_object";
    
    auto deserialized = serializer.deserialize(invalid_json);
    EXPECT_FALSE(deserialized.has_value());
}

// Tests for std::map<std::string, T> specialization
TEST(JsonSerializerContainerTest, MapIntSerializeDeserialize) {
    JsonSerializer<std::map<std::string, int>> serializer;
    std::map<std::string, int> original = {{"key1", 1}, {"key2", 2}, {"key3", 3}};
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_object());
    EXPECT_EQ(serialized.size(), 3);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, MapStringSerializeDeserialize) {
    JsonSerializer<std::map<std::string, std::string>> serializer;
    std::map<std::string, std::string> original = {{"name", "John"}, {"city", "NYC"}};
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_object());
    EXPECT_EQ(serialized.size(), 2);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, MapEmptySerializeDeserialize) {
    JsonSerializer<std::map<std::string, int>> serializer;
    std::map<std::string, int> original;
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_object());
    EXPECT_EQ(serialized.size(), 0);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, MapDeserializeInvalidJson) {
    JsonSerializer<std::map<std::string, int>> serializer;
    nlohmann::json invalid_json = "not_an_object";
    
    auto deserialized = serializer.deserialize(invalid_json);
    EXPECT_FALSE(deserialized.has_value());
}

// Nested container tests
TEST(JsonSerializerContainerTest, VectorOfVectorSerializeDeserialize) {
    JsonSerializer<std::vector<std::vector<int>>> serializer;
    std::vector<std::vector<int>> original = {{1, 2}, {3, 4, 5}, {}};
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_array());
    EXPECT_EQ(serialized.size(), 3);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}

TEST(JsonSerializerContainerTest, MapOfVectorSerializeDeserialize) {
    JsonSerializer<std::map<std::string, std::vector<int>>> serializer;
    std::map<std::string, std::vector<int>> original = {{"nums1", {1, 2, 3}}, {"nums2", {4, 5}}};
    
    auto serialized = serializer.serialize(original);
    EXPECT_TRUE(serialized.is_object());
    EXPECT_EQ(serialized.size(), 2);
    
    auto deserialized = serializer.deserialize(serialized);
    EXPECT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized.value(), original);
}
