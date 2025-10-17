# WIP GUI Widgets Library
cmake_minimum_required(VERSION 3.20)

project(wip_gui_widgets
    VERSION 1.0.0
    DESCRIPTION "GUI Widgets library for WIP framework"
    LANGUAGES CXX
)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(WIDGET_SOURCES
    src/widget.cpp
    src/panel.cpp
    src/selectable_text_widget.cpp
)

# Header files
set(WIDGET_HEADERS
    include/widget.h
    include/panel.h
    include/selectable_text_widget.h
    include/widgets.h
)

# Create the library
add_library(wip_gui_widgets STATIC ${WIDGET_SOURCES} ${WIDGET_HEADERS})

# Set include directories
target_include_directories(wip_gui_widgets
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(wip_gui_widgets
    PUBLIC
        imgui
)

# Compiler-specific options
target_compile_features(wip_gui_widgets PUBLIC cxx_std_17)

# Set target properties
set_target_properties(wip_gui_widgets PROPERTIES
    OUTPUT_NAME "wip_gui_widgets"
    DEBUG_POSTFIX "_d"
)

# Enable warnings
if(MSVC)
    target_compile_options(wip_gui_widgets PRIVATE /W4)
else()
    target_compile_options(wip_gui_widgets PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Display configuration info
message(STATUS "WIP GUI Widgets Library configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Source files: ${WIDGET_SOURCES}")
message(STATUS "  Header files: ${WIDGET_HEADERS}")

# Add the library to the parent project if this is a subdirectory
if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(STATUS "WIP GUI Widgets library added as subdirectory")
endif()