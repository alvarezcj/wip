# Event System Library
#
# Strongly-typed event system for C++ applications with support for:
# - Type-safe event dispatching and handling
# - Priority-based event processing
# - Thread-safe operations
# - Automatic resource management with RAII
# - Event filtering and subscription management

cmake_minimum_required(VERSION 3.20)

# Find required packages
find_package(Threads REQUIRED)

# Create the event library
add_library(wip_utils_event
    src/event_dispatcher.cpp
)

# Set target properties
target_compile_features(wip_utils_event PUBLIC cxx_std_17)

# Include directories
target_include_directories(wip_utils_event
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link required system libraries
target_link_libraries(wip_utils_event
    PUBLIC
        Threads::Threads
)

# Set library properties
set_target_properties(wip_utils_event PROPERTIES
    OUTPUT_NAME "wip_utils_event"
    VERSION 1.0.0
    SOVERSION 1
)

# Export the target
set_property(TARGET wip_utils_event PROPERTY EXPORT_NAME event)

# Add alias for consistent naming
add_library(wip::utils::event ALIAS wip_utils_event)

# Install rules
install(TARGETS wip_utils_event
    EXPORT wip_utils_event_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/wip/utils/event
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(EXPORT wip_utils_event_targets
    FILE wip_utils_event_targets.cmake
    NAMESPACE wip::utils::
    DESTINATION lib/cmake/wip_utils_event
)

# Create config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/wip_utils_event_config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/wip_utils_event_config.cmake"
    INSTALL_DESTINATION lib/cmake/wip_utils_event
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/wip_utils_event_config_version.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/wip_utils_event_config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/wip_utils_event_config_version.cmake"
    DESTINATION lib/cmake/wip_utils_event
)

# Tests
if(BUILD_TESTING)
    add_subdirectory(test)
endif()