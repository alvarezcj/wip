find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

add_executable(gran_azul src/main.cpp)

target_link_libraries(gran_azul PRIVATE 
    wip::gui::application
    wip::gui::window
    wip::utils::event
    wip::utils::process
    OpenGL::GL
    imgui::imgui
    ${X11_LIBRARIES}
)

# Set output directory
set_target_properties(gran_azul PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)