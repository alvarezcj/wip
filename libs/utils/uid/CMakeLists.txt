# Create library
add_library(wip_utils_uid STATIC)
target_sources(wip_utils_uid PRIVATE src/uid.cpp)
target_include_directories(wip_utils_uid PUBLIC include)
target_compile_features(wip_utils_uid PUBLIC cxx_std_17)

# Create alias for easier linking
add_library(wip::utils::uid ALIAS wip_utils_uid)

# Add tests if enabled
if(BUILD_TESTS)
    add_executable(test_wip_utils_uid test/test_uid.cpp)
    target_link_libraries(test_wip_utils_uid PRIVATE 
        wip::utils::uid 
        GTest::gtest_main
    )
    
    # Add test to CTest
    add_test(NAME test_wip_utils_uid COMMAND test_wip_utils_uid)
endif()