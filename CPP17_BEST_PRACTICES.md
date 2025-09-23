# C++17 Best Practices Guide

This guide outlines best practices for modern C++17 development, covering coding conventions, design patterns, testing strategies, and performance considerations.

## Table of Contents
- [Coding Conventions](#coding-conventions)
- [Modern C++17 Features](#modern-c17-features)
- [Testing Best Practices](#testing-best-practices)
- [Memory Management](#memory-management)
- [Error Handling](#error-handling)
- [Performance Guidelines](#performance-guidelines)
- [Code Organization](#code-organization)
- [Documentation Standards](#documentation-standards)
- [Build and Tooling](#build-and-tooling)

## Coding Conventions

### Naming Conventions

#### Variables and Functions
```cpp
// ✅ Good: snake_case for variables and functions
int user_count = 0;
std::string file_name = "data.txt";
void calculate_average();
bool is_valid_input();

// ❌ Bad: camelCase or PascalCase for variables
int userCount = 0;          // Don't use
std::string FileName = "";  // Don't use
```

#### Classes and Types
```cpp
// ✅ Good: PascalCase for classes, structs, enums
class UserManager {
public:
    enum class Status {
        Active,
        Inactive,
        Pending
    };
};

struct ConfigData {
    std::string host;
    int port;
};

// ✅ Good: Type aliases
using UserId = std::uint64_t;
using UserMap = std::unordered_map<UserId, std::string>;
```

#### Constants and Macros
```cpp
// ✅ Good: SCREAMING_SNAKE_CASE for constants
constexpr int MAX_BUFFER_SIZE = 1024;
constexpr double PI = 3.14159265359;

// ✅ Good: Prefer constexpr over #define
constexpr auto DEFAULT_TIMEOUT = std::chrono::seconds{30};

// ❌ Avoid macros when possible
#define MAX_SIZE 1024  // Use constexpr instead
```

#### Namespaces
```cpp
// ✅ Good: snake_case for namespaces
namespace wip::utils::string {
    // Implementation
}

namespace wip::network::http {
    // Implementation
}

// ✅ Good: Namespace aliases for long names
namespace json = nlohmann;
namespace fs = std::filesystem;
```

### File Organization

#### Header Files
```cpp
#pragma once

// 1. Standard library includes (sorted alphabetically)
#include <memory>
#include <string>
#include <vector>

// 2. Third-party library includes
#include <nlohmann/json.hpp>

// 3. Project includes
#include "base_class.h"
#include "config.h"

namespace wip::utils::file {

/**
 * @brief File utility class for common file operations
 */
class FileManager {
public:
    // Public types first
    enum class Mode {
        Read,
        Write,
        Append
    };

    // Constructors
    explicit FileManager(std::string_view filename);
    
    // Destructor
    ~FileManager() = default;
    
    // Copy/Move operations (Rule of 5)
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
    FileManager(FileManager&&) = default;
    FileManager& operator=(FileManager&&) = default;

    // Public methods
    bool open(Mode mode);
    void close();
    std::optional<std::string> read_line();

private:
    // Private members
    std::string filename_;
    std::unique_ptr<std::ifstream> file_stream_;
    
    // Private methods
    void validate_filename() const;
};

}  // namespace wip::utils::file
```

#### Implementation Files
```cpp
#include "file_manager.h"

#include <fstream>
#include <stdexcept>

namespace wip::utils::file {

FileManager::FileManager(std::string_view filename) 
    : filename_{filename} {
    validate_filename();
}

bool FileManager::open(Mode mode) {
    // Implementation with clear error handling
    try {
        // ... implementation
        return true;
    }
    catch (const std::exception& e) {
        // Log error appropriately
        return false;
    }
}

void FileManager::validate_filename() const {
    if (filename_.empty()) {
        throw std::invalid_argument("Filename cannot be empty");
    }
    // Additional validation
}

}  // namespace wip::utils::file
```

## Modern C++17 Features

### Structured Bindings
```cpp
// ✅ Good: Use structured bindings for multiple return values
std::pair<bool, std::string> parse_config() {
    // ... implementation
    return {true, "config parsed"};
}

// Usage
auto [success, message] = parse_config();
if (success) {
    std::cout << message << std::endl;
}

// ✅ Good: With containers
std::map<std::string, int> counters;
for (const auto& [key, value] : counters) {
    std::cout << key << ": " << value << std::endl;
}
```

### std::optional for Optional Values
```cpp
// ✅ Good: Use std::optional instead of nullable pointers
std::optional<User> find_user(UserId id) {
    auto it = users_.find(id);
    if (it != users_.end()) {
        return it->second;
    }
    return std::nullopt;
}

// Usage
if (auto user = find_user(123)) {
    std::cout << "Found user: " << user->name << std::endl;
} else {
    std::cout << "User not found" << std::endl;
}
```

### std::string_view for Efficient String Handling
```cpp
// ✅ Good: Use string_view for read-only string parameters
void process_text(std::string_view text) {
    // No copying, efficient for large strings
    if (text.empty()) return;
    
    // Use string_view methods
    auto pos = text.find("pattern");
    auto substr = text.substr(0, 10);
}

// ✅ Good: Return string_view when returning substrings of existing strings
class ConfigParser {
private:
    std::string config_data_;
    
public:
    std::string_view get_section(std::string_view section_name) const {
        // Return view into config_data_
        // ... implementation
    }
};
```

### if constexpr for Compile-time Branching
```cpp
template<typename T>
void print_value(const T& value) {
    if constexpr (std::is_arithmetic_v<T>) {
        std::cout << "Number: " << value << std::endl;
    } else if constexpr (std::is_same_v<T, std::string>) {
        std::cout << "String: " << value << std::endl;
    } else {
        std::cout << "Other type" << std::endl;
    }
}
```

### Filesystem Library
```cpp
#include <filesystem>

namespace fs = std::filesystem;

// ✅ Good: Use filesystem for path operations
bool create_directory_structure(const fs::path& base_path) {
    try {
        fs::create_directories(base_path / "logs");
        fs::create_directories(base_path / "config");
        fs::create_directories(base_path / "data");
        return true;
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }
}

// ✅ Good: Iterate through directories
void process_files(const fs::path& directory) {
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            process_file(entry.path());
        }
    }
}
```

### Class Template Argument Deduction (CTAD)
```cpp
// ✅ Good: Let compiler deduce template arguments
std::vector data{1, 2, 3, 4, 5};  // deduces std::vector<int>
std::pair config{"hostname", 8080};  // deduces std::pair<const char*, int>

// ✅ Good: With smart pointers
auto ptr = std::make_unique<FileManager>("config.txt");
auto shared = std::make_shared<DatabaseConnection>(connection_string);
```

## Testing Best Practices

### Test Structure and Organization

#### Test File Organization
```cpp
#include "user_manager.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Test fixture for related tests
class UserManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
        manager_ = std::make_unique<UserManager>();
        test_user_ = User{"john_doe", "john@example.com", 25};
    }
    
    void TearDown() override {
        // Cleanup after each test
        manager_.reset();
    }
    
    // Helper methods for tests
    User create_test_user(const std::string& name) {
        return User{name, name + "@example.com", 20};
    }
    
    // Test data
    std::unique_ptr<UserManager> manager_;
    User test_user_;
};
```

#### Test Naming and Structure
```cpp
// ✅ Good: Descriptive test names following Given_When_Then pattern
TEST_F(UserManagerTest, AddUser_WhenValidUser_ReturnsTrue) {
    // Arrange
    const auto user = create_test_user("alice");
    
    // Act
    const bool result = manager_->add_user(user);
    
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(manager_->get_user_count(), 1);
}

TEST_F(UserManagerTest, AddUser_WhenDuplicateUser_ReturnsFalse) {
    // Arrange
    manager_->add_user(test_user_);
    
    // Act
    const bool result = manager_->add_user(test_user_);
    
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(manager_->get_user_count(), 1);
}

// ✅ Good: Test edge cases
TEST_F(UserManagerTest, FindUser_WhenEmptyManager_ReturnsNullopt) {
    // Act
    const auto result = manager_->find_user("nonexistent");
    
    // Assert
    EXPECT_FALSE(result.has_value());
}
```

#### Parameterized Tests
```cpp
// ✅ Good: Use parameterized tests for testing multiple inputs
class StringUtilsTest : public ::testing::TestWithParam<std::pair<std::string, std::string>> {
};

TEST_P(StringUtilsTest, ToUpper_VariousInputs_ReturnsExpectedOutput) {
    // Arrange
    const auto [input, expected] = GetParam();
    
    // Act
    const auto result = wip::utils::string::to_upper(input);
    
    // Assert
    EXPECT_EQ(result, expected);
}

INSTANTIATE_TEST_SUITE_P(
    VariousStrings,
    StringUtilsTest,
    ::testing::Values(
        std::make_pair("hello", "HELLO"),
        std::make_pair("WORLD", "WORLD"),
        std::make_pair("", ""),
        std::make_pair("Hello World!", "HELLO WORLD!")
    )
);
```

#### Mock Objects with Google Mock
```cpp
// Mock interface
class MockFileSystem {
public:
    MOCK_METHOD(bool, file_exists, (const std::string& path), (const));
    MOCK_METHOD(std::optional<std::string>, read_file, (const std::string& path), (const));
    MOCK_METHOD(bool, write_file, (const std::string& path, const std::string& content), (const));
};

// Test using mock
TEST(ConfigManagerTest, LoadConfig_WhenFileExists_ReturnsConfig) {
    // Arrange
    MockFileSystem mock_fs;
    ConfigManager manager{&mock_fs};
    
    const std::string config_content = R"({"setting": "value"})";
    
    EXPECT_CALL(mock_fs, file_exists("config.json"))
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(mock_fs, read_file("config.json"))
        .WillOnce(::testing::Return(config_content));
    
    // Act
    const auto config = manager.load_config("config.json");
    
    // Assert
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->get_setting("setting"), "value");
}
```

### Test Coverage and Quality

```cpp
// ✅ Good: Test both success and failure paths
TEST_F(DatabaseTest, Connect_WhenValidConnectionString_ReturnsTrue) {
    // Test successful connection
}

TEST_F(DatabaseTest, Connect_WhenInvalidConnectionString_ReturnsFalse) {
    // Test connection failure
}

TEST_F(DatabaseTest, Connect_WhenDatabaseUnavailable_ThrowsException) {
    // Test exception handling
}

// ✅ Good: Test boundary conditions
TEST_F(VectorUtilsTest, FindElement_EmptyVector_ReturnsNullopt) {
    std::vector<int> empty_vec;
    EXPECT_FALSE(find_element(empty_vec, 42).has_value());
}

TEST_F(VectorUtilsTest, FindElement_SingleElement_Found) {
    std::vector<int> single_vec{42};
    EXPECT_EQ(find_element(single_vec, 42), 0);
}
```

## Memory Management

### Smart Pointers Usage

```cpp
// ✅ Good: Prefer smart pointers over raw pointers
class ResourceManager {
private:
    std::unique_ptr<Database> database_;
    std::shared_ptr<Logger> logger_;
    std::vector<std::unique_ptr<Worker>> workers_;

public:
    explicit ResourceManager(std::shared_ptr<Logger> logger)
        : logger_{std::move(logger)}
        , database_{std::make_unique<Database>()}
    {
        // Initialize workers
        for (int i = 0; i < 4; ++i) {
            workers_.push_back(std::make_unique<Worker>(i));
        }
    }
    
    // ✅ Good: Return unique_ptr for transferring ownership
    std::unique_ptr<Task> create_task(const TaskConfig& config) {
        return std::make_unique<Task>(config, logger_);
    }
    
    // ✅ Good: Accept shared_ptr by const reference when not storing
    void process_with_logger(const std::shared_ptr<Logger>& external_logger) {
        external_logger->log("Processing...");
    }
};
```

### RAII (Resource Acquisition Is Initialization)

```cpp
// ✅ Good: RAII for automatic resource management
class FileHandle {
private:
    std::unique_ptr<std::FILE, decltype(&std::fclose)> file_;

public:
    explicit FileHandle(const std::string& filename)
        : file_{std::fopen(filename.c_str(), "r"), &std::fclose}
    {
        if (!file_) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
    }
    
    // Automatic cleanup via destructor
    ~FileHandle() = default;
    
    // Non-copyable, movable
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    FileHandle(FileHandle&&) = default;
    FileHandle& operator=(FileHandle&&) = default;
    
    std::string read_line() {
        std::string line;
        char buffer[1024];
        if (std::fgets(buffer, sizeof(buffer), file_.get())) {
            line = buffer;
        }
        return line;
    }
};

// Usage
void process_file(const std::string& filename) {
    try {
        FileHandle file{filename};  // RAII - automatic cleanup
        auto line = file.read_line();
        // ... process line
    } // File automatically closed here
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
```

## Error Handling

### Exception Safety

```cpp
// ✅ Good: Exception-safe function with strong guarantee
class SafeContainer {
private:
    std::vector<std::string> data_;
    
public:
    void add_items(const std::vector<std::string>& items) {
        // Create temporary copy first
        auto temp_data = data_;
        
        try {
            // Modify temporary
            temp_data.reserve(temp_data.size() + items.size());
            temp_data.insert(temp_data.end(), items.begin(), items.end());
            
            // Commit changes (strong exception safety)
            data_ = std::move(temp_data);
        }
        catch (const std::exception&) {
            // Original data unchanged
            throw;
        }
    }
};
```

### Error Handling with std::optional and std::expected (C++23 preview)

```cpp
// ✅ Good: Use std::optional for operations that may fail
std::optional<User> parse_user(std::string_view json_str) {
    try {
        auto json = nlohmann::json::parse(json_str);
        if (!json.contains("name") || !json.contains("email")) {
            return std::nullopt;
        }
        
        return User{
            json["name"].get<std::string>(),
            json["email"].get<std::string>(),
            json.value("age", 0)
        };
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

// ✅ Good: Custom error types for specific domains
enum class ParseError {
    InvalidJson,
    MissingField,
    InvalidType
};

// Simple Result type (until std::expected is available)
template<typename T, typename E>
class Result {
private:
    std::variant<T, E> data_;
    
public:
    Result(T value) : data_{std::move(value)} {}
    Result(E error) : data_{std::move(error)} {}
    
    bool has_value() const { return std::holds_alternative<T>(data_); }
    const T& value() const { return std::get<T>(data_); }
    const E& error() const { return std::get<E>(data_); }
};

Result<User, ParseError> parse_user_with_error(std::string_view json_str) {
    try {
        auto json = nlohmann::json::parse(json_str);
        if (!json.contains("name")) {
            return ParseError::MissingField;
        }
        
        return User{json["name"], json["email"], json.value("age", 0)};
    }
    catch (const nlohmann::json::parse_error&) {
        return ParseError::InvalidJson;
    }
}
```

## Performance Guidelines

### Avoid Unnecessary Copies

```cpp
// ✅ Good: Pass large objects by const reference
void process_large_data(const std::vector<ComplexObject>& data) {
    for (const auto& item : data) {  // const reference in range-for
        // Process item
    }
}

// ✅ Good: Move when transferring ownership
std::vector<std::string> create_strings() {
    std::vector<std::string> result;
    result.reserve(1000);  // Reserve capacity
    
    for (int i = 0; i < 1000; ++i) {
        result.emplace_back("String " + std::to_string(i));  // Construct in-place
    }
    
    return result;  // Automatic move (NRVO)
}

// ✅ Good: Perfect forwarding for templates
template<typename T>
void add_to_container(std::vector<T>& container, T&& item) {
    container.emplace_back(std::forward<T>(item));
}
```

### Container Optimizations

```cpp
// ✅ Good: Reserve capacity when size is known
std::vector<int> process_data(size_t count) {
    std::vector<int> result;
    result.reserve(count);  // Avoid reallocations
    
    for (size_t i = 0; i < count; ++i) {
        result.push_back(static_cast<int>(i * 2));
    }
    
    return result;
}

// ✅ Good: Use appropriate container for use case
class StringIndex {
private:
    // Fast lookups
    std::unordered_map<std::string, size_t> name_to_index_;
    // Preserve order
    std::vector<std::string> index_to_name_;
    
public:
    size_t add_string(std::string name) {
        auto it = name_to_index_.find(name);
        if (it != name_to_index_.end()) {
            return it->second;
        }
        
        size_t index = index_to_name_.size();
        index_to_name_.push_back(name);
        name_to_index_.emplace(std::move(name), index);
        return index;
    }
};
```

## Code Organization

### Header-Only vs. Compiled Libraries

```cpp
// ✅ Good: Header-only for templates and small utilities
// math_utils.h
#pragma once

namespace wip::utils::math {

template<typename T>
constexpr T clamp(T value, T min_val, T max_val) {
    return (value < min_val) ? min_val : (value > max_val) ? max_val : value;
}

template<typename T>
constexpr T square(T value) {
    return value * value;
}

}  // namespace wip::utils::math
```

### Dependency Management

```cpp
// ✅ Good: Use forward declarations to reduce compile dependencies
// user_manager.h
#pragma once

#include <memory>
#include <string>

// Forward declarations
class Database;
class Logger;
struct User;

class UserManager {
private:
    std::unique_ptr<Database> database_;
    std::shared_ptr<Logger> logger_;
    
public:
    explicit UserManager(std::shared_ptr<Logger> logger);
    ~UserManager();  // Need definition in .cpp for unique_ptr
    
    bool add_user(const User& user);
    std::optional<User> find_user(const std::string& name) const;
};
```

## Documentation Standards

### Doxygen Comments

```cpp
/**
 * @brief Manages user accounts and authentication
 * 
 * The UserManager class provides a high-level interface for user management
 * operations including registration, authentication, and profile updates.
 * 
 * @example
 * ```cpp
 * auto manager = std::make_unique<UserManager>(logger);
 * if (manager->register_user("john", "password123")) {
 *     auto user = manager->authenticate("john", "password123");
 * }
 * ```
 */
class UserManager {
public:
    /**
     * @brief Registers a new user account
     * 
     * @param username The unique username for the account
     * @param password The password for the account (will be hashed)
     * @param email Optional email address for the account
     * 
     * @return true if registration successful, false if username exists
     * 
     * @throws std::invalid_argument if username or password is empty
     * @throws DatabaseException if database operation fails
     * 
     * @pre username must not be empty
     * @pre password must be at least 8 characters
     * @post User is added to database if successful
     */
    bool register_user(const std::string& username, 
                      const std::string& password,
                      const std::optional<std::string>& email = std::nullopt);
    
    /**
     * @brief Authenticates a user with username and password
     * 
     * @param username The username to authenticate
     * @param password The password to verify
     * 
     * @return User object if authentication successful, std::nullopt otherwise
     * 
     * @complexity O(1) average case for username lookup
     */
    std::optional<User> authenticate(const std::string& username,
                                   const std::string& password) const;
};
```

## Build and Tooling

### CMake Best Practices

```cmake
# Set minimum version and policies
cmake_minimum_required(VERSION 3.15)

# Enable modern policies
cmake_policy(SET CMP0077 NEW)  # option() honors normal variables

project(WIP 
    VERSION 1.0.0
    DESCRIPTION "Work In Progress C++ Library"
    LANGUAGES CXX
)

# Global settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler-specific settings
if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    
    # Additional useful warnings
    add_compile_options(
        -Wconversion
        -Wsign-conversion
        -Wunused
        -Wcast-align
        -Wcast-qual
        -Wold-style-cast
    )
endif()

# Debug/Release specific settings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Enable different sanitizers for debug builds
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if(ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()
```

### Static Analysis Integration

```cmake
# Find and enable clang-tidy
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY 
        ${CLANG_TIDY_EXE};
        -checks=-*,readability-*,performance-*,modernize-*,bugprone-*;
        -header-filter=.*
    )
endif()

# Find and enable cppcheck
find_program(CPPCHECK_EXE NAMES "cppcheck")
if(CPPCHECK_EXE)
    set(CMAKE_CXX_CPPCHECK 
        ${CPPCHECK_EXE};
        --enable=warning,performance,portability;
        --std=c++17;
        --template=gcc;
        --inline-suppr;
        --quiet
    )
endif()
```

### Code Formatting

Create `.clang-format` file:
```yaml
---
Language: Cpp
Standard: c++17
BasedOnStyle: Google
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100
BreakBeforeBraces: Attach
AllowShortFunctionsOnASingleLine: Inline
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
AlwaysBreakTemplateDeclarations: Yes
BreakConstructorInitializers: BeforeColon
ConstructorInitializerIndentWidth: 4
ContinuationIndentWidth: 4
Cpp11BracedListStyle: true
PointerAlignment: Left
SpaceAfterCStyleCast: false
SpaceBeforeParens: ControlStatements
SpacesInParentheses: false
SpacesInSquareBrackets: false
```

This comprehensive guide covers the essential best practices for modern C++17 development. Following these guidelines will help ensure your code is maintainable, performant, and follows industry standards.
