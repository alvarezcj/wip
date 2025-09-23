# Create library
add_library(wip_utils_string STATIC)
target_sources(wip_utils_string PRIVATE src/string.cpp)
target_include_directories(wip_utils_string PUBLIC include)
target_compile_features(wip_utils_string PUBLIC cxx_std_17)

# Create alias for easier linking
add_library(wip::utils::string ALIAS wip_utils_string)

# Add tests if enabled
if(BUILD_TESTS)
    add_executable(test_wip_utils_string test/test_string.cpp)
    target_link_libraries(test_wip_utils_string PRIVATE 
        wip::utils::string 
        GTest::gtest_main
    )
    
    # Add test to CTest
    add_test(NAME test_wip_utils_string COMMAND test_wip_utils_string)
endif()
