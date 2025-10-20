# Create library
add_library(wip_analysis STATIC)

# Add source files
target_sources(wip_analysis PRIVATE
    # Core classes
    src/analysis_types.cpp
    src/tool_config.cpp
    src/analysis_tool.cpp
    src/analysis_engine.cpp
    
    # Tool implementations
    src/tools/cppcheck_tool.cpp
    src/tools/clang_tidy_tool.cpp
)

# Set include directories
target_include_directories(wip_analysis PUBLIC include)

# Set C++ standard
target_compile_features(wip_analysis PUBLIC cxx_std_17)

# Link dependencies
target_link_libraries(wip_analysis 
    PUBLIC
        nlohmann_json::nlohmann_json
        wip::utils::process
    PRIVATE
        # Private dependencies if any
)

# Create alias for easier linking
add_library(wip::analysis ALIAS wip_analysis)

# Add tests if enabled
if(BUILD_TESTS)
    add_executable(test_wip_analysis
        test/test_analysis_types.cpp
        test/test_tool_config.cpp
        test/test_analysis_tool.cpp
        test/test_cppcheck_tool.cpp
        test/test_clang_tidy_tool.cpp
        test/test_analysis_engine.cpp
    )
    
    target_link_libraries(test_wip_analysis PRIVATE 
        wip::analysis 
        GTest::gtest_main
    )
    
    # Add test to CTest
    add_test(NAME test_wip_analysis COMMAND test_wip_analysis)
endif()