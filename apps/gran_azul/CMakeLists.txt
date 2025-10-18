find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

add_executable(gran_azul 
    src/main.cpp
    src/widgets/cppcheck_config_widget.cpp
    src/widgets/log_window_panel.cpp
    src/widgets/analysis_result.cpp
    src/widgets/analysis_result_panel.cpp
    src/widgets/path_selector_widget.cpp
    src/widgets/progress_dialog.cpp
    src/utils/async_process_executor.cpp
    src/project/project_config.cpp
    src/project/project_manager.cpp
    src/report/report_generator.cpp
)

target_link_libraries(gran_azul PRIVATE 
    wip::gui::application
    wip::gui::window
    wip::utils::event
    wip::utils::process
    wip::gui::widgets
    wip::serialization::json_serializer
    nfd
    OpenGL::GL
    imgui::imgui
    ${X11_LIBRARIES}
)

# Add include directories for local widgets and project
target_include_directories(gran_azul PRIVATE
    src/widgets
    src/project
    src/report
)

# Set output directory
set_target_properties(gran_azul PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)