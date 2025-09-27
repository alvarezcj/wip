# Create test_gui executable - GLFW + ImGui desktop application demo
add_executable(test_gui)
target_sources(test_gui PRIVATE src/main.cpp)
target_include_directories(test_gui PRIVATE include)

# Link required libraries
target_link_libraries(test_gui PRIVATE 
    wip::gui::window
    imgui::imgui
    wip::utils::string
    wip::utils::rng
    wip::utils::event
)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux OpenGL and X11 libraries
    find_package(OpenGL REQUIRED)
    target_link_libraries(test_gui PRIVATE
        OpenGL::GL
        ${CMAKE_DL_LIBS}
        pthread
    )
elseif(WIN32)
    # Windows libraries
    target_link_libraries(test_gui PRIVATE
        opengl32
        gdi32
        user32
        kernel32
    )
endif()

message(STATUS "test_gui application configured")