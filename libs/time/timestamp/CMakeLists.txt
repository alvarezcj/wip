cmake_minimum_required(VERSION 3.20)
project(wip_time_timestamp)

# Create the timestamp library
add_library(wip_time_timestamp STATIC
    src/timestamp.cpp
)

# Set target properties
target_include_directories(wip_time_timestamp 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link against the time utilities library
target_link_libraries(wip_time_timestamp 
    PUBLIC 
        wip::time::utilities
)

# Set C++ standard
target_compile_features(wip_time_timestamp PUBLIC cxx_std_17)

# Alias for better namespacing
add_library(wip::time::timestamp ALIAS wip_time_timestamp)

# Enable testing if this is the main project or testing is enabled
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()