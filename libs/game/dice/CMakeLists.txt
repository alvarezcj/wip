# Create dice library
add_library(wip_game_dice STATIC
    src/dice.cpp
)

# Set the alias for consistent naming
add_library(wip::game::dice ALIAS wip_game_dice)

# Include directories for the library
target_include_directories(wip_game_dice PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Set C++ standard
target_compile_features(wip_game_dice PUBLIC cxx_std_17)

# Link dependencies
target_link_libraries(wip_game_dice PUBLIC
    wip::utils::rng
)

# Create tests if testing is enabled
if(BUILD_TESTS)
    add_executable(test_wip_game_dice
        test/test_dice.cpp
    )
    
    target_link_libraries(test_wip_game_dice
        wip_game_dice
        GTest::gtest_main
    )
    
    add_test(NAME test_wip_game_dice COMMAND test_wip_game_dice)
endif()