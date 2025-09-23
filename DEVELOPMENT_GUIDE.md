# WIP C++ Development Guide

This guide explains how to add new libraries and applications to the WIP C++ project following best practices for C++17 development.

## Project Structure Overview

```
WIP/
├── CMakeLists.txt          # Root CMake configuration
├── apps/                   # Applications
│   └── playground/         # Example application
├── libs/                   # Libraries organized by domain
│   └── utils/             # Utility libraries domain
│       └── file/          # File utilities library
├── third_party/           # External dependencies
└── build/                 # Build output directory
```

## Adding a New Library

### 1. Directory Structure

To create a new library, follow this structure under `libs/`:

```
libs/
└── <domain>/              # Domain name (e.g., utils, core, network)
    └── <library_name>/    # Library name (e.g., file, string, math)
        ├── CMakeLists.txt
        ├── include/
        │   └── <library_name>.h
        ├── src/
        │   └── <library_name>.cpp
        └── test/
            └── test_<library_name>.cpp
```

### 2. Step-by-Step Instructions

#### Step 1: Create Directory Structure
```bash
# Example: Creating a new string utility library
mkdir -p libs/utils/string/{include,src,test}
```

#### Step 2: Create Header File (`include/<library_name>.h`)

**Template:**
```cpp
#pragma once

#include <string>
#include <vector>

namespace wip::utils::<library_name> {

/**
 * @brief Brief description of the function
 * @param param1 Description of parameter
 * @return Description of return value
 */
std::string example_function(const std::string& input);

/**
 * @brief Another example function
 * @param data Input data
 * @return Processed data
 */
std::vector<std::string> split(const std::string& data, char delimiter);

}  // namespace wip::utils::<library_name>
```

**Example (`libs/utils/string/include/string.h`):**
```cpp
#pragma once

#include <string>
#include <vector>

namespace wip::utils::string {

/**
 * @brief Converts a string to uppercase
 * @param input The input string
 * @return The uppercase version of the input string
 */
std::string to_upper(const std::string& input);

/**
 * @brief Splits a string by delimiter
 * @param data The string to split
 * @param delimiter The character to split by
 * @return Vector of string parts
 */
std::vector<std::string> split(const std::string& data, char delimiter);

}  // namespace wip::utils::string
```

#### Step 3: Create Implementation File (`src/<library_name>.cpp`)

**Template:**
```cpp
#include "<library_name>.h"

#include <algorithm>
#include <sstream>

namespace wip::utils::<library_name> {

std::string example_function(const std::string& input) {
    // Implementation here
    return input;
}

std::vector<std::string> split(const std::string& data, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(data);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    return result;
}

}  // namespace wip::utils::<library_name>
```

#### Step 4: Create Test File (`test/test_<library_name>.cpp`)

**Template:**
```cpp
#include "<library_name>.h"

#include <gtest/gtest.h>

class <LibraryName>Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here
    }
    
    void TearDown() override {
        // Cleanup code here
    }
};

TEST_F(<LibraryName>Test, ExampleFunction) {
    // Arrange
    std::string input = "test";
    std::string expected = "test";
    
    // Act
    std::string result = wip::utils::<library_name>::example_function(input);
    
    // Assert
    EXPECT_EQ(result, expected);
}

TEST_F(<LibraryName>Test, SplitFunction) {
    // Arrange
    std::string data = "a,b,c";
    char delimiter = ',';
    std::vector<std::string> expected = {"a", "b", "c"};
    
    // Act
    auto result = wip::utils::<library_name>::split(data, delimiter);
    
    // Assert
    EXPECT_EQ(result, expected);
}
```

#### Step 5: Create CMakeLists.txt

**Template (`libs/<domain>/<library_name>/CMakeLists.txt`):**
```cmake
# Create library
add_library(wip_<domain>_<library_name> STATIC)
target_sources(wip_<domain>_<library_name> PRIVATE src/<library_name>.cpp)
target_include_directories(wip_<domain>_<library_name> PUBLIC include)

# Set C++17 standard for this target
target_compile_features(wip_<domain>_<library_name> PUBLIC cxx_std_17)

# Create alias for easier linking
add_library(wip::<domain>::<library_name> ALIAS wip_<domain>_<library_name>)

# Add tests if enabled
if(BUILD_TESTS)
    add_executable(test_wip_<domain>_<library_name> test/test_<library_name>.cpp)
    target_link_libraries(test_wip_<domain>_<library_name> PRIVATE 
        wip::<domain>::<library_name> 
        GTest::gtest_main
    )
    
    # Add test to CTest
    add_test(NAME test_wip_<domain>_<library_name> COMMAND test_wip_<domain>_<library_name>)
endif()
```

#### Step 6: Update Root CMakeLists.txt

Add your new library to the root `CMakeLists.txt`:

```cmake
# wip libraries
add_subdirectory(libs/utils/file)
add_subdirectory(libs/utils/string)  # Add this line
```

## Adding a New Application

### 1. Directory Structure

```
apps/
└── <app_name>/            # Application name
    ├── CMakeLists.txt
    ├── include/           # Private headers (optional)
    │   └── <app_name>.h
    └── src/
        └── main.cpp
```

### 2. Step-by-Step Instructions

#### Step 1: Create Directory Structure
```bash
# Example: Creating a new calculator application
mkdir -p apps/calculator/{include,src}
```

#### Step 2: Create Main File (`src/main.cpp`)

**Template:**
```cpp
#include <iostream>
#include <string>

// Include your libraries
#include "file.h"  // Example library include

int main(int argc, char* argv[]) {
    try {
        std::cout << "Starting <app_name> application..." << std::endl;
        
        // Application logic here
        
        std::cout << "<app_name> completed successfully." << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

#### Step 3: Create CMakeLists.txt (`apps/<app_name>/CMakeLists.txt`)

**Template:**
```cmake
# Create application executable
add_executable(<app_name>)
target_sources(<app_name> PRIVATE src/main.cpp)

# Set C++17 standard
target_compile_features(<app_name> PRIVATE cxx_std_17)

# Include directories (if you have private headers)
target_include_directories(<app_name> PRIVATE include)

# Link libraries
target_link_libraries(<app_name> PRIVATE 
    wip::<domain>::<library_name>
    # Add other libraries as needed
    # nlohmann_json::nlohmann_json
)

# Enable all warnings and treat them as errors (best practice)
if(MSVC)
    target_compile_options(<app_name> PRIVATE /W4 /WX)
else()
    target_compile_options(<app_name> PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
```

#### Step 4: Update Root CMakeLists.txt

Add your new application to the root `CMakeLists.txt`:

```cmake
# Apps
add_subdirectory(apps/playground)
add_subdirectory(apps/calculator)  # Add this line
```

## Building and Testing

### Build Commands
```bash
# Configure and build
mkdir -p build
cd build
cmake ..
make -j$(nproc)

# Or using cmake --build
cmake --build . -j$(nproc)
```

### Running Tests
```bash
# Run all tests
ctest --output-on-failure

# Run specific test
./bin/test_wip_utils_string
```

### Running Applications
```bash
# Run from build directory
./bin/<app_name>

# Or from project root
./build/bin/<app_name>
```

## Best Practices

### Code Organization
1. **Namespace Convention**: Use `wip::<domain>::<library_name>` for libraries
2. **File Naming**: Use snake_case for files and directories
3. **Header Guards**: Always use `#pragma once`
4. **Documentation**: Use Doxygen-style comments for public APIs

### CMake Best Practices
1. **Modern CMake**: Use target-based approach with `target_*` commands
2. **Aliases**: Create aliases for libraries to make linking easier
3. **Standards**: Explicitly set C++17 standard for each target
4. **Warnings**: Enable comprehensive warnings and treat them as errors

### Testing Best Practices
1. **Test Structure**: Use AAA pattern (Arrange, Act, Assert)
2. **Test Classes**: Use test fixtures for complex setups
3. **Naming**: Use descriptive test names that explain what is being tested
4. **Coverage**: Aim for high test coverage of public APIs

### C++17 Features to Leverage
- `std::string_view` for efficient string handling
- Structured bindings for multiple return values
- `if constexpr` for compile-time branching
- `std::optional` for optional return values
- `std::filesystem` for file operations
- Range-based for loops with structured bindings

## Example: Complete String Library

Here's a complete example of adding a string utility library:

1. **Create structure**: `mkdir -p libs/utils/string/{include,src,test}`

2. **Header** (`libs/utils/string/include/string.h`):
```cpp
#pragma once

#include <string>
#include <vector>
#include <string_view>

namespace wip::utils::string {

std::string to_upper(std::string_view input);
std::string to_lower(std::string_view input);
std::vector<std::string> split(std::string_view data, char delimiter);
std::string trim(std::string_view input);

}  // namespace wip::utils::string
```

3. **Implementation** (`libs/utils/string/src/string.cpp`):
```cpp
#include "string.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace wip::utils::string {

std::string to_upper(std::string_view input) {
    std::string result{input};
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string to_lower(std::string_view input) {
    std::string result{input};
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::vector<std::string> split(std::string_view data, char delimiter) {
    std::vector<std::string> result;
    std::string item;
    std::stringstream ss{std::string{data}};
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    return result;
}

std::string trim(std::string_view input) {
    const auto start = input.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) return "";
    
    const auto end = input.find_last_not_of(" \t\n\r");
    return std::string{input.substr(start, end - start + 1)};
}

}  // namespace wip::utils::string
```

4. **Tests** (`libs/utils/string/test/test_string.cpp`):
```cpp
#include "string.h"

#include <gtest/gtest.h>

class StringTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(StringTest, ToUpper) {
    EXPECT_EQ(wip::utils::string::to_upper("hello"), "HELLO");
    EXPECT_EQ(wip::utils::string::to_upper(""), "");
    EXPECT_EQ(wip::utils::string::to_upper("Hello World"), "HELLO WORLD");
}

TEST_F(StringTest, ToLower) {
    EXPECT_EQ(wip::utils::string::to_lower("HELLO"), "hello");
    EXPECT_EQ(wip::utils::string::to_lower(""), "");
    EXPECT_EQ(wip::utils::string::to_lower("Hello World"), "hello world");
}

TEST_F(StringTest, Split) {
    auto result = wip::utils::string::split("a,b,c", ',');
    std::vector<std::string> expected = {"a", "b", "c"};
    EXPECT_EQ(result, expected);
    
    result = wip::utils::string::split("", ',');
    expected = {""};
    EXPECT_EQ(result, expected);
}

TEST_F(StringTest, Trim) {
    EXPECT_EQ(wip::utils::string::trim("  hello  "), "hello");
    EXPECT_EQ(wip::utils::string::trim("hello"), "hello");
    EXPECT_EQ(wip::utils::string::trim("   "), "");
    EXPECT_EQ(wip::utils::string::trim(""), "");
}
```

5. **CMakeLists.txt** (`libs/utils/string/CMakeLists.txt`):
```cmake
add_library(wip_utils_string STATIC)
target_sources(wip_utils_string PRIVATE src/string.cpp)
target_include_directories(wip_utils_string PUBLIC include)
target_compile_features(wip_utils_string PUBLIC cxx_std_17)

add_library(wip::utils::string ALIAS wip_utils_string)

if(BUILD_TESTS)
    add_executable(test_wip_utils_string test/test_string.cpp)
    target_link_libraries(test_wip_utils_string PRIVATE 
        wip::utils::string 
        GTest::gtest_main
    )
    add_test(NAME test_wip_utils_string COMMAND test_wip_utils_string)
endif()
```

6. **Update root CMakeLists.txt**:
```cmake
# wip libraries
add_subdirectory(libs/utils/file)
add_subdirectory(libs/utils/string)
```

This guide provides a solid foundation for expanding your C++ project while maintaining consistency and following modern C++17 best practices.
