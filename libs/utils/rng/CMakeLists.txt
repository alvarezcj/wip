cmake_minimum_required(VERSION 3.14)

# Create the library
add_library(wip_utils_rng STATIC
    src/rng.cpp
)

# Set the alias for consistent naming
add_library(wip::utils::rng ALIAS wip_utils_rng)

# Include directories for the library
target_include_directories(wip_utils_rng PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Set C++ standard
target_compile_features(wip_utils_rng PUBLIC cxx_std_17)

# Link threading library if needed (for thread_local support)
find_package(Threads REQUIRED)
target_link_libraries(wip_utils_rng PRIVATE Threads::Threads)

# Create tests if testing is enabled
if(BUILD_TESTS)
    add_executable(test_wip_utils_rng
        test/test_rng.cpp
    )
    
    target_link_libraries(test_wip_utils_rng
        wip_utils_rng
        GTest::gtest_main
    )
    
    add_test(NAME test_wip_utils_rng COMMAND test_wip_utils_rng)
endif()