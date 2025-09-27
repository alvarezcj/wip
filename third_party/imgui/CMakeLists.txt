include(FetchContent)

# Download ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking  # Docking branch with mature docking support
    GIT_SHALLOW TRUE
)

# Make ImGui available
FetchContent_MakeAvailable(imgui)

# Create ImGui library target since it doesn't provide CMake configuration
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

# Set up include directories
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Disable Wayland support to avoid dependencies
target_compile_definitions(imgui PUBLIC 
    GLFW_INCLUDE_NONE=1
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
)

# Add platform-specific compile definitions
if(UNIX AND NOT APPLE)
    target_compile_definitions(imgui PUBLIC _GLFW_X11)
endif()

# Check for OpenGL headers
find_path(OPENGL_INCLUDE_DIR GL/gl.h
    PATHS /usr/include /usr/local/include
)

# Check for Vulkan support
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    message(STATUS "Vulkan SDK found: ${Vulkan_LIBRARY}")
else()
    message(STATUS "Vulkan SDK not found")
endif()

# Only add GLFW and OpenGL backends if dependencies are available
if(TARGET glfw AND OPENGL_INCLUDE_DIR)
    target_sources(imgui PRIVATE
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    
    message(STATUS "ImGui GLFW and OpenGL3 backends enabled")
else()
    message(STATUS "ImGui GLFW/OpenGL3 backends disabled (missing dependencies)")
endif()

# Add Vulkan backend if available
if(TARGET glfw AND Vulkan_FOUND)
    target_sources(imgui PRIVATE
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    )
    message(STATUS "ImGui GLFW and Vulkan backends enabled")
else()
    message(STATUS "ImGui Vulkan backend disabled (missing dependencies)")
endif()

# Link dependencies
if(TARGET glfw)
    target_link_libraries(imgui PUBLIC glfw)
endif()

# Find OpenGL for OpenGL3 backend
find_package(OpenGL QUIET)
if(OpenGL_FOUND)
    target_link_libraries(imgui PUBLIC OpenGL::GL)
    message(STATUS "OpenGL support enabled for ImGui")
else()
    message(STATUS "OpenGL not found - ImGui will have limited functionality")
endif()

# Link Vulkan if available
if(Vulkan_FOUND)
    target_link_libraries(imgui PUBLIC Vulkan::Vulkan)
    message(STATUS "Vulkan support enabled for ImGui")
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Required system libraries for Linux
    target_link_libraries(imgui PRIVATE dl pthread)
endif()

# Create convenience alias
add_library(imgui::imgui ALIAS imgui)

message(STATUS "ImGui configured successfully with GLFW and OpenGL3 backends")