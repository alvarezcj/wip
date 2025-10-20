cmake_minimum_required(VERSION 3.20)

# Test executable for time utilities
add_executable(test_wip_time_utilities
    test_time_utilities.cpp
)

target_link_libraries(test_wip_time_utilities
    PRIVATE
        wip::time::utilities
        GTest::gtest_main
)

target_compile_features(test_wip_time_utilities PRIVATE cxx_std_17)

# Add test to CTest
add_test(NAME TimeUtilitiesTest COMMAND test_wip_time_utilities)