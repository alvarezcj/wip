#include "json_serializer.h"

namespace wip::serialization {

// String specialization implementation
template<>
nlohmann::json JsonSerializer<std::string>::serialize(const std::string& data) const {
    return nlohmann::json(data);
}

template<>
JsonSerializer<std::string>::DeserializeResult JsonSerializer<std::string>::deserialize(const nlohmann::json& serialized_data) const {
    if (serialized_data.is_string()) {
        return serialized_data.get<std::string>();
    }
    return std::nullopt;
}

// Integer specialization implementation
template<>
nlohmann::json JsonSerializer<int>::serialize(const int& data) const {
    return nlohmann::json(data);
}

template<>
JsonSerializer<int>::DeserializeResult JsonSerializer<int>::deserialize(const nlohmann::json& serialized_data) const {
    if (serialized_data.is_number_integer()) {
        return serialized_data.get<int>();
    }
    return std::nullopt;
}

// Double specialization implementation
template<>
nlohmann::json JsonSerializer<double>::serialize(const double& data) const {
    return nlohmann::json(data);
}

template<>
JsonSerializer<double>::DeserializeResult JsonSerializer<double>::deserialize(const nlohmann::json& serialized_data) const {
    if (serialized_data.is_number_float() || serialized_data.is_number_integer()) {
        return serialized_data.get<double>();
    }
    return std::nullopt;
}

// Boolean specialization implementation
template<>
nlohmann::json JsonSerializer<bool>::serialize(const bool& data) const {
    return nlohmann::json(data);
}

template<>
JsonSerializer<bool>::DeserializeResult JsonSerializer<bool>::deserialize(const nlohmann::json& serialized_data) const {
    if (serialized_data.is_boolean()) {
        return serialized_data.get<bool>();
    }
    return std::nullopt;
}

}  // namespace wip::serialization