#include "uid.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <chrono>

namespace wip::utils::uid {

// Static random number generator
static thread_local std::mt19937 generator(
    static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    )
);

// Default constructor - generates a new random UUID
UID::UID() : uuid_data_(generate_random_data()) {
}

// Constructor from existing UUID data
UID::UID(const UUIDData& data) : uuid_data_(data) {
}

// Constructor from UUID string representation
UID::UID(const std::string& uuid_string) : uuid_data_(parse_uuid_string(uuid_string)) {
}

// Rule of 7: Copy constructor
UID::UID(const UID& other) : uuid_data_(other.uuid_data_) {
}

// Rule of 7: Copy assignment operator
UID& UID::operator=(const UID& other) {
    if (this != &other) {
        uuid_data_ = other.uuid_data_;
    }
    return *this;
}

// Rule of 7: Move constructor
UID::UID(UID&& other) noexcept : uuid_data_(std::move(other.uuid_data_)) {
}

// Rule of 7: Move assignment operator
UID& UID::operator=(UID&& other) noexcept {
    if (this != &other) {
        uuid_data_ = std::move(other.uuid_data_);
    }
    return *this;
}

// Static factory method to create a new random UID
UID UID::generate() {
    return UID();
}

// Convert UID to standard UUID string representation
std::string UID::to_string() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    
    // Format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    // Positions: 0-3, 4-5, 6-7, 8-9, 10-15
    for (size_t i = 0; i < 16; ++i) {
        // Add hyphens at appropriate positions
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            oss << '-';
        }
        oss << std::setw(2) << static_cast<unsigned int>(uuid_data_[i]);
    }
    
    return oss.str();
}

// Get the raw UUID data
const UID::UUIDData& UID::data() const {
    return uuid_data_;
}

// Check if UID is null (all zeros)
bool UID::is_null() const {
    for (const auto& byte : uuid_data_) {
        if (byte != 0) {
            return false;
        }
    }
    return true;
}

// Create a null UID (all zeros)
UID UID::null() {
    UUIDData null_data{};
    null_data.fill(0);
    return UID(null_data);
}

// Comparison operators
bool UID::operator==(const UID& other) const {
    return uuid_data_ == other.uuid_data_;
}

bool UID::operator!=(const UID& other) const {
    return !(*this == other);
}

// Private: Generate random UUID data
UID::UUIDData UID::generate_random_data() {
    std::uniform_int_distribution<uint8_t> distribution(0, 255);
    UUIDData data;
    
    // Generate 16 random bytes
    for (size_t i = 0; i < 16; ++i) {
        data[i] = distribution(generator);
    }
    
    // Set version to 4 (random UUID)
    // Version bits are in the most significant 4 bits of byte 6
    data[6] = (data[6] & 0x0F) | 0x40;
    
    // Set variant bits (RFC 4122)
    // Variant bits are in the most significant 2 bits of byte 8
    data[8] = (data[8] & 0x3F) | 0x80;
    
    return data;
}

// Private: Parse UUID string into byte array
UID::UUIDData UID::parse_uuid_string(const std::string& uuid_string) {
    if (!is_valid_uuid_string(uuid_string)) {
        throw std::invalid_argument("Invalid UUID string format");
    }
    
    UUIDData data;
    size_t data_index = 0;
    
    for (size_t i = 0; i < uuid_string.length(); ++i) {
        char c = uuid_string[i];
        
        // Skip hyphens
        if (c == '-') {
            continue;
        }
        
        // Parse pairs of hex characters
        if (data_index < 16 && i + 1 < uuid_string.length()) {
            char next_c = uuid_string[i + 1];
            if (next_c != '-') {
                uint8_t high_nibble = hex_char_to_byte(c);
                uint8_t low_nibble = hex_char_to_byte(next_c);
                data[data_index] = (high_nibble << 4) | low_nibble;
                ++data_index;
                ++i; // Skip the next character since we processed it
            }
        }
    }
    
    return data;
}

// Private: Validate UUID string format
bool UID::is_valid_uuid_string(const std::string& uuid_string) {
    // UUID format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx (36 characters)
    if (uuid_string.length() != 36) {
        return false;
    }
    
    // Check hyphen positions
    if (uuid_string[8] != '-' || uuid_string[13] != '-' || 
        uuid_string[18] != '-' || uuid_string[23] != '-') {
        return false;
    }
    
    // Check that all other characters are hex digits
    for (size_t i = 0; i < uuid_string.length(); ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            continue; // Skip hyphens
        }
        
        char c = uuid_string[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    
    return true;
}

// Private: Convert hex character to byte value
uint8_t UID::hex_char_to_byte(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    } else if (c >= 'a' && c <= 'f') {
        return static_cast<uint8_t>(c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
        return static_cast<uint8_t>(c - 'A' + 10);
    } else {
        throw std::invalid_argument("Invalid hex character");
    }
}

// Private: Convert byte value to hex character
char UID::byte_to_hex_char(uint8_t value) {
    if (value < 10) {
        return static_cast<char>('0' + value);
    } else if (value < 16) {
        return static_cast<char>('a' + value - 10);
    } else {
        throw std::invalid_argument("Invalid byte value for hex conversion");
    }
}

}  // namespace wip::utils::uid

// Hash function specialization for std::hash
namespace std {

size_t hash<wip::utils::uid::UID>::operator()(const wip::utils::uid::UID& uid) const noexcept {
    // Use the first 8 bytes as hash input for efficiency
    const auto& data = uid.data();
    size_t hash_value = 0;
    
    // Combine first 8 bytes using a simple hash combining algorithm
    for (size_t i = 0; i < 8 && i < data.size(); ++i) {
        hash_value ^= std::hash<uint8_t>{}(data[i]) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
    }
    
    return hash_value;
}

}  // namespace std