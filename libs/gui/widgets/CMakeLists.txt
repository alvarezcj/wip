# Library target
add_library(wip_gui_widgets STATIC)
target_sources(wip_gui_widgets PRIVATE 
    src/widget.cpp
    src/panel.cpp
    src/selectable_text_widget.cpp
)
target_include_directories(wip_gui_widgets PUBLIC include)
target_compile_features(wip_gui_widgets PUBLIC cxx_std_17)

# Link required libraries
target_link_libraries(wip_gui_widgets PUBLIC 
    imgui
)

# Alias for easier linking
add_library(wip::gui::widgets ALIAS wip_gui_widgets)

# Tests (when test file exists)
if(BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test/test_widgets.cpp)
    add_executable(test_wip_gui_widgets 
        test/test_widgets.cpp
    )
    target_link_libraries(test_wip_gui_widgets PRIVATE 
        wip::gui::widgets
        GTest::gtest_main
    )
    add_test(NAME test_wip_gui_widgets COMMAND test_wip_gui_widgets)
endif()