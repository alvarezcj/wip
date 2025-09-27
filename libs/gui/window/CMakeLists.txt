# Library target
add_library(wip_gui_window STATIC)
target_sources(wip_gui_window PRIVATE src/window.cpp)
target_include_directories(wip_gui_window PUBLIC include)
target_compile_features(wip_gui_window PUBLIC cxx_std_17)

# Link required libraries
target_link_libraries(wip_gui_window PUBLIC 
    wip::utils::event
    glfw
)

# Alias for easier linking
add_library(wip::gui::window ALIAS wip_gui_window)

# Tests
if(BUILD_TESTS)
    add_executable(test_wip_gui_window 
        test/test_window.cpp
        test/test_window_events.cpp
    )
    target_link_libraries(test_wip_gui_window PRIVATE 
        wip::gui::window
        GTest::gtest_main
    )
    add_test(NAME test_wip_gui_window COMMAND test_wip_gui_window)
endif()