// Wrapper to include GLFW native without Wayland dependencies
#ifndef IMGUI_GLFW_NO_WAYLAND_WRAPPER_H
#define IMGUI_GLFW_NO_WAYLAND_WRAPPER_H

// Disable Wayland support in GLFW native headers
#define _GLFW_WAYLAND 0
#undef _GLFW_WAYLAND

// Only enable X11 support
#define _GLFW_X11 1

// Include GLFW headers
#include <GLFW/glfw3.h>

// Only include X11 parts of the native header
#ifdef GLFW_EXPOSE_NATIVE_X11
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <GL/glx.h>

extern "C" {
GLFWAPI Display* glfwGetX11Display(void);
GLFWAPI Window glfwGetX11Window(GLFWwindow* window);
GLFWAPI GLXContext glfwGetGLXContext(GLFWwindow* window);
GLFWAPI GLXWindow glfwGetGLXWindow(GLFWwindow* window);
}
#endif

#endif // IMGUI_GLFW_NO_WAYLAND_WRAPPER_H