cmake_minimum_required(VERSION 3.10)

project(WIP CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
endif()

# Third-party libraries
add_subdirectory(third_party/nlohmann_json)
add_subdirectory(third_party/gtest)

# wip libraries
add_subdirectory(libs/cli/args)
add_subdirectory(libs/utils/file)
add_subdirectory(libs/utils/rng)
add_subdirectory(libs/utils/string)
add_subdirectory(libs/utils/uid)
add_subdirectory(libs/serialization/serializer)
add_subdirectory(libs/serialization/json_serializer)

# Apps
add_subdirectory(apps/playground)