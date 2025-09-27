# Library target
add_library(wip_gui_application STATIC)
target_sources(wip_gui_application PRIVATE 
    src/application.cpp
    src/layer.cpp
)
target_include_directories(wip_gui_application PUBLIC include)
target_compile_features(wip_gui_application PUBLIC cxx_std_17)

# Link required libraries
target_link_libraries(wip_gui_application PUBLIC 
    wip::gui::window
    wip::utils::event
    imgui::imgui
)

# Alias for easier linking
add_library(wip::gui::application ALIAS wip_gui_application)

# Tests
if(BUILD_TESTS)
    add_executable(test_wip_gui_application 
        test/test_application.cpp
    )
    target_link_libraries(test_wip_gui_application PRIVATE 
        wip::gui::application
        GTest::gtest_main
    )
    add_test(NAME test_wip_gui_application COMMAND test_wip_gui_application)
endif()