cmake_minimum_required(VERSION 3.20)
project(wip_time)

# Add the time domain libraries
add_subdirectory(utilities)
add_subdirectory(timestamp)

# Create an umbrella target that includes both libraries
add_library(wip_time INTERFACE)

target_link_libraries(wip_time 
    INTERFACE 
        wip::time::utilities
        wip::time::timestamp
)

# Alias for better namespacing
add_library(wip::time ALIAS wip_time)