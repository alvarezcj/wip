# Create library
add_library(wip_utils_process STATIC)
target_sources(wip_utils_process PRIVATE src/process.cpp)
target_include_directories(wip_utils_process PUBLIC include)
target_compile_features(wip_utils_process PUBLIC cxx_std_17)

# Link required system libraries for process management
target_link_libraries(wip_utils_process PRIVATE pthread)

# Create alias for easier linking
add_library(wip::utils::process ALIAS wip_utils_process)

# Add tests if enabled
if(BUILD_TESTS)
    add_executable(test_wip_utils_process test/test_process.cpp)
    target_link_libraries(test_wip_utils_process PRIVATE 
        wip::utils::process 
        GTest::gtest_main
    )
    
    # Add test to CTest
    add_test(NAME test_wip_utils_process COMMAND test_wip_utils_process)
endif()