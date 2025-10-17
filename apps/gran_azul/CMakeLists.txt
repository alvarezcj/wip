find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

add_executable(gran_azul 
    src/main.cpp
    src/widgets/cppcheck_config_widget.cpp
    src/widgets/log_window_panel.cpp
    src/widgets/analysis_result.cpp
    src/widgets/analysis_result_panel.cpp
)

target_link_libraries(gran_azul PRIVATE 
    wip::gui::application
    wip::gui::window
    wip::utils::event
    wip::utils::process
    wip_gui_widgets
    OpenGL::GL
    imgui::imgui
    ${X11_LIBRARIES}
)

# Add include directories for local widgets
target_include_directories(gran_azul PRIVATE
    src/widgets
)

# Set output directory
set_target_properties(gran_azul PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)