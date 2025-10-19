# Analysis Library CMakeLists.txt

# Create the analysis library
add_library(wip_analysis
    # Core classes
    src/analysis_types.cpp
    src/tool_config.cpp
    src/analysis_tool.cpp
    src/analysis_engine.cpp
    
    # Tool implementations
    src/tools/cppcheck_tool.cpp
    src/tools/clang_tidy_tool.cpp
)

# Set target properties
target_compile_features(wip_analysis PUBLIC cxx_std_17)

# Include directories
target_include_directories(wip_analysis 
    PUBLIC 
        include
    PRIVATE 
        src
)

# Link dependencies
target_link_libraries(wip_analysis
    PUBLIC
        nlohmann_json::nlohmann_json
        wip_utils_process  # For process execution
    PRIVATE
        # Any private dependencies
)

# Compiler warnings
if(MSVC)
    target_compile_options(wip_analysis PRIVATE /W4 /WX)
else()
    target_compile_options(wip_analysis PRIVATE 
        -Wall -Wextra -Werror -pedantic
        -Wno-unused-parameter  # Allow unused parameters in virtual functions
    )
endif()

# Create alias for easier linking
add_library(wip::analysis ALIAS wip_analysis)

# Install rules (optional)
if(WIP_INSTALL_LIBS)
    install(TARGETS wip_analysis
        EXPORT wip_analysis_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
    
    install(DIRECTORY include/
        DESTINATION include/wip/analysis
        FILES_MATCHING PATTERN "*.h"
    )
endif()

# Tests (if enabled) - TODO: Create test files
if(BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test/test_analysis_types.cpp)
    add_executable(test_wip_analysis
        test/test_analysis_types.cpp
        test/test_tool_config.cpp
        test/test_analysis_tool.cpp
        test/test_cppcheck_tool.cpp
        test/test_clang_tidy_tool.cpp
        test/test_analysis_engine.cpp
    )
    
    target_link_libraries(test_wip_analysis
        wip_analysis
        gtest_main
    )
    
    target_compile_features(test_wip_analysis PRIVATE cxx_std_17)
    
    # Add test to CTest
    add_test(NAME test_wip_analysis COMMAND test_wip_analysis)
    
    # Set test properties for better output
    set_tests_properties(test_wip_analysis PROPERTIES
        TIMEOUT 300
        LABELS "unit;analysis"
    )
endif()