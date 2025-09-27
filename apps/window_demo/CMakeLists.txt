# Window Demo Application
add_executable(window_demo
    src/main.cpp
)

# Link required libraries
target_link_libraries(window_demo PRIVATE 
    wip::gui::window
    wip::utils::event
)

# Platform-specific libraries for OpenGL
if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(window_demo PRIVATE
        OpenGL::GL
        ${CMAKE_DL_LIBS}
        pthread
    )
elseif(WIN32)
    target_link_libraries(window_demo PRIVATE
        opengl32
        gdi32
        user32
        kernel32
    )
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(window_demo PRIVATE
        OpenGL::GL
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# Set C++17 standard
target_compile_features(window_demo PRIVATE cxx_std_17)

message(STATUS "window_demo application configured")