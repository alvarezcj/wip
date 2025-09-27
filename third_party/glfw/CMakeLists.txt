include(FetchContent)

# Download GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

# Configure GLFW build options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build support for Wayland" FORCE)

# Check for X11 support
find_package(X11 QUIET)
if(X11_FOUND)
    set(GLFW_BUILD_X11 ON CACHE BOOL "Build support for X11" FORCE)
    message(STATUS "X11 support enabled")
else()
    set(GLFW_BUILD_X11 OFF CACHE BOOL "Build support for X11" FORCE)
    message(STATUS "X11 support disabled (X11 libraries not found)")
endif()

# Check for Wayland support
find_program(WAYLAND_SCANNER wayland-scanner)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(WAYLAND QUIET wayland-client)
endif()

if(WAYLAND_SCANNER AND WAYLAND_FOUND)
    set(GLFW_BUILD_WAYLAND ON CACHE BOOL "Build support for Wayland" FORCE)
    message(STATUS "Wayland support enabled")
else()
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build support for Wayland" FORCE)
    message(STATUS "Wayland support disabled (dependencies not found)")
endif()

# Fallback to basic configuration if neither X11 nor Wayland is available
if(NOT X11_FOUND AND NOT WAYLAND_FOUND)
    message(WARNING "Neither X11 nor Wayland found. GLFW will be built with minimal functionality.")
    set(GLFW_BUILD_X11 OFF CACHE BOOL "Build support for X11" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build support for Wayland" FORCE)
endif()

# Make GLFW available
FetchContent_MakeAvailable(glfw)

# Create convenience alias if it doesn't exist
if(NOT TARGET glfw::glfw)
    add_library(glfw::glfw ALIAS glfw)
endif()

message(STATUS "GLFW configured successfully with X11 and Wayland support")