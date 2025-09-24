#pragma once

#include <array>
#include <string>
#include <random>
#include <functional>

namespace wip::utils::uid {

/**
 * @brief Universally Unique Identifier (UUID) class
 * 
 * This class represents a 128-bit UUID that can be generated using random numbers
 * and converted to a standard string representation (RFC 4122 format).
 * Implements the Rule of 7 for proper resource management.
 */
class UID {
public:
    // Type alias for the underlying UUID data (128 bits = 16 bytes)
    using UUIDData = std::array<uint8_t, 16>;
    
    // Default constructor - generates a new random UUID
    UID();
    
    // Constructor from existing UUID data
    explicit UID(const UUIDData& data);
    
    // Constructor from UUID string representation
    explicit UID(const std::string& uuid_string);
    
    // Rule of 7: Copy constructor
    UID(const UID& other);
    
    // Rule of 7: Copy assignment operator
    UID& operator=(const UID& other);
    
    // Rule of 7: Move constructor
    UID(UID&& other) noexcept;
    
    // Rule of 7: Move assignment operator
    UID& operator=(UID&& other) noexcept;
    
    // Rule of 7: Destructor
    ~UID() = default;
    
    /**
     * @brief Static factory method to create a new random UID
     * @return A new UID with random data
     */
    static UID generate();
    
    /**
     * @brief Convert UID to standard UUID string representation
     * @return String in format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
     */
    std::string to_string() const;
    
    /**
     * @brief Get the raw UUID data
     * @return Reference to the underlying byte array
     */
    const UUIDData& data() const;
    
    /**
     * @brief Check if UID is null (all zeros)
     * @return true if all bytes are zero, false otherwise
     */
    bool is_null() const;
    
    /**
     * @brief Create a null UID (all zeros)
     * @return A UID with all bytes set to zero
     */
    static UID null();
    
    // Comparison operators
    bool operator==(const UID& other) const;
    bool operator!=(const UID& other) const;

private:
    UUIDData uuid_data_;
    
    /**
     * @brief Generate random UUID data
     * @return Array of 16 random bytes formatted as UUID v4
     */
    static UUIDData generate_random_data();
    
    /**
     * @brief Parse UUID string into byte array
     * @param uuid_string String in UUID format
     * @return Parsed byte array
     * @throws std::invalid_argument if string format is invalid
     */
    static UUIDData parse_uuid_string(const std::string& uuid_string);
    
    /**
     * @brief Validate UUID string format
     * @param uuid_string String to validate
     * @return true if valid UUID format, false otherwise
     */
    static bool is_valid_uuid_string(const std::string& uuid_string);
    
    /**
     * @brief Convert hex character to byte value
     * @param c Hex character ('0'-'9', 'A'-'F', 'a'-'f')
     * @return Byte value (0-15)
     * @throws std::invalid_argument if character is not valid hex
     */
    static uint8_t hex_char_to_byte(char c);
    
    /**
     * @brief Convert byte value to hex character
     * @param value Byte value (0-15)
     * @return Hex character ('0'-'9', 'a'-'f')
     */
    static char byte_to_hex_char(uint8_t value);
};

}  // namespace wip::utils::uid

// Hash function specialization for std::hash
namespace std {
template<>
struct hash<wip::utils::uid::UID> {
    size_t operator()(const wip::utils::uid::UID& uid) const noexcept;
};
}  // namespace std