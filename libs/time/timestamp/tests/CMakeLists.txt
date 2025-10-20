cmake_minimum_required(VERSION 3.20)

# Test executable for timestamp
add_executable(test_wip_time_timestamp
    test_timestamp.cpp
)

target_link_libraries(test_wip_time_timestamp
    PRIVATE
        wip::time::timestamp
        GTest::gtest_main
)

target_compile_features(test_wip_time_timestamp PRIVATE cxx_std_17)

# Add test to CTest
add_test(NAME TimestampTest COMMAND test_wip_time_timestamp)