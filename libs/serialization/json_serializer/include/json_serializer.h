#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include "serializer.h"

namespace wip::serialization {

// JSON serializer alias for convenience
template<typename T>
using JsonSerializer = Serializer<nlohmann::json, T>;

// String specialization declarations
template<>
nlohmann::json JsonSerializer<std::string>::serialize(const std::string& data) const;
template<>
JsonSerializer<std::string>::DeserializeResult JsonSerializer<std::string>::deserialize(const nlohmann::json& serialized_data) const;

// Integer specialization declarations
template<>
nlohmann::json JsonSerializer<int>::serialize(const int& data) const;
template<>
JsonSerializer<int>::DeserializeResult JsonSerializer<int>::deserialize(const nlohmann::json& serialized_data) const;

// Double specialization declarations
template<>
nlohmann::json JsonSerializer<double>::serialize(const double& data) const;
template<>
JsonSerializer<double>::DeserializeResult JsonSerializer<double>::deserialize(const nlohmann::json& serialized_data) const;

// Boolean specialization declarations
template<>
nlohmann::json JsonSerializer<bool>::serialize(const bool& data) const;
template<>
JsonSerializer<bool>::DeserializeResult JsonSerializer<bool>::deserialize(const nlohmann::json& serialized_data) const;

// Partial specialization for std::vector<T>
template<typename T>
class Serializer<nlohmann::json, std::vector<T>> {
public:
    using DeserializeResult = std::optional<std::vector<T>>;

    nlohmann::json serialize(const std::vector<T>& data) const {
        nlohmann::json result = nlohmann::json::array();
        std::transform(data.begin(), data.end(), std::back_inserter(result),
                       [](const T& element) { return JsonSerializer<T>().serialize(element); });
        return result;
    }

    DeserializeResult deserialize(const nlohmann::json& serialized_data) const {
        if (!serialized_data.is_array()) {
            return std::nullopt;
        }

        std::vector<T> result;
        result.reserve(serialized_data.size());
        std::transform(serialized_data.begin(), serialized_data.end(), std::back_inserter(result),
                       [](const nlohmann::json& element) { return JsonSerializer<T>().deserialize(element).value_or(T{}); });
        return result;
    }
};

// Partial specialization for std::unordered_map<std::string, T>
template<typename T>
class Serializer<nlohmann::json, std::unordered_map<std::string, T>> {
public:
    using DeserializeResult = std::optional<std::unordered_map<std::string, T>>;

    nlohmann::json serialize(const std::unordered_map<std::string, T>& data) const {
        nlohmann::json result = nlohmann::json::object();
        for (const auto& [key, value] : data) {
            result[key] = JsonSerializer<T>().serialize(value);
        }
        return result;
    }

    DeserializeResult deserialize(const nlohmann::json& serialized_data) const {
        if (!serialized_data.is_object()) {
            return std::nullopt;
        }

        std::unordered_map<std::string, T> result;
        for (const auto& [key, value] : serialized_data.items()) {
            auto deserialized_value = JsonSerializer<T>().deserialize(value);
            if (!deserialized_value) {
                return std::nullopt;
            }
            result[key] = *deserialized_value;
        }
        return result;
    }
};

// Partial specialization for std::map<std::string, T>
template<typename T>
class Serializer<nlohmann::json, std::map<std::string, T>> {
public:
    using DeserializeResult = std::optional<std::map<std::string, T>>;

    nlohmann::json serialize(const std::map<std::string, T>& data) const {
        nlohmann::json result = nlohmann::json::object();
        for (const auto& [key, value] : data) {
            result[key] = JsonSerializer<T>().serialize(value);
        }
        return result;
    }

    DeserializeResult deserialize(const nlohmann::json& serialized_data) const {
        if (!serialized_data.is_object()) {
            return std::nullopt;
        }

        std::map<std::string, T> result;
        for (const auto& [key, value] : serialized_data.items()) {
            auto deserialized_value = JsonSerializer<T>().deserialize(value);
            if (!deserialized_value) {
                return std::nullopt;
            }
            result[key] = *deserialized_value;
        }
        return result;
    }
};
}  // namespace wip::serialization
