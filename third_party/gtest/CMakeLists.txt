# GoogleTest Library Configuration
cmake_minimum_required(VERSION 3.20)

# Only configure GoogleTest if testing is enabled
if(NOT BUILD_TESTS)
    message(STATUS "Testing is disabled, skipping GoogleTest configuration")
    return()
endif()

include(FetchContent)

# Download GoogleTest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Configure GoogleTest build options
set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# Disable GoogleTest's own tests
set(gtest_build_tests OFF CACHE BOOL "Build gtest's sample programs" FORCE)
set(gtest_build_samples OFF CACHE BOOL "Build gtest's sample programs" FORCE)

# Make GoogleTest available
FetchContent_MakeAvailable(googletest)

# Create alias targets for consistency with find_package(GTest)
if(NOT TARGET GTest::gtest)
    add_library(GTest::gtest ALIAS gtest)
endif()

if(NOT TARGET GTest::gtest_main)
    add_library(GTest::gtest_main ALIAS gtest_main)
endif()

# Set properties for better integration
set_target_properties(gtest PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

set_target_properties(gtest_main PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Verify GoogleTest targets are available
if(NOT TARGET gtest)
    message(FATAL_ERROR "GoogleTest gtest target not found")
endif()

if(NOT TARGET gtest_main)
    message(FATAL_ERROR "GoogleTest gtest_main target not found")
endif()

message(STATUS "GoogleTest configured successfully")
message(STATUS "  - GTest::gtest: Available")
message(STATUS "  - GTest::gtest_main: Available")
message(STATUS "  - Version: 1.14.0")