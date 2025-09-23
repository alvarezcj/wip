#pragma once
#include <optional>

namespace wip::serialization {

/**
 * @brief Serializer interface for different data formats
 * Template class that should be specialized for specific combinations
 */
template<typename SerializationType, typename DataType>
class Serializer {
public:
    using DeserializeResult = std::optional<DataType>;
    
    /**
     * @brief serialize data to serialization type
     * @param data The data to serialize
     * @return Serialized representation
     */
    SerializationType serialize(const DataType& data) const;
    
    /**
     * @brief deserialize serialization type to data
     * @param serialized_data The serialized representation
     * @return Deserialized data or nullopt if parsing fails
     */
    DeserializeResult deserialize(const SerializationType& serialized_data) const;
};

}  // namespace wip::serialization
