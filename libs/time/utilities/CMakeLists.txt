cmake_minimum_required(VERSION 3.20)
project(wip_time_utilities)

# Create the utilities library
add_library(wip_time_utilities STATIC
    src/time_utilities.cpp
)

# Set target properties
target_include_directories(wip_time_utilities 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set C++ standard
target_compile_features(wip_time_utilities PUBLIC cxx_std_17)

# Alias for better namespacing
add_library(wip::time::utilities ALIAS wip_time_utilities)

# Enable testing if this is the main project or testing is enabled
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()