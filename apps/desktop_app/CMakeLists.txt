find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

add_executable(desktop_app src/main.cpp)

target_link_libraries(desktop_app PRIVATE 
    wip::gui::application
    wip::gui::window
    wip::utils::event
    OpenGL::GL
    imgui::imgui
    ${X11_LIBRARIES}
)

# Set output directory
set_target_properties(desktop_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)