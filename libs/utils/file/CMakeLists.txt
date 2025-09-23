# Create library
add_library(wip_utils_file STATIC)
target_sources(wip_utils_file PRIVATE src/file.cpp)
target_include_directories(wip_utils_file PUBLIC include)

add_library(wip::utils::file ALIAS wip_utils_file)

if(BUILD_TESTS)
    # Add test executable
    add_executable(test_wip_utils_file test/test_file.cpp)
    target_link_libraries(test_wip_utils_file PRIVATE wip::utils::file GTest::gtest_main)

    # Add test to CTest
    add_test(NAME test_wip_utils_file COMMAND test_wip_utils_file)
endif()