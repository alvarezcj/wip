# CLI Args Library CMakeLists.txt

# Library target
add_library(wip_cli_args
    src/args.cpp
)

target_include_directories(wip_cli_args PUBLIC
    include
)

target_compile_features(wip_cli_args PUBLIC cxx_std_17)

# Set compiler warnings
if(MSVC)
    target_compile_options(wip_cli_args PRIVATE /W4 /WX)
else()
    target_compile_options(wip_cli_args PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

# Test executable
if(BUILD_TESTING)
    add_executable(test_wip_cli_args
        test/test_args.cpp
    )

    target_link_libraries(test_wip_cli_args
        wip_cli_args
        gtest_main
    )

    target_compile_features(test_wip_cli_args PRIVATE cxx_std_17)

    # Add test to CTest
    add_test(NAME test_wip_cli_args COMMAND test_wip_cli_args)
endif()