#include "window.h"
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace wip::gui::window {

// Static member definitions
int Window::glfw_window_count_ = 0;
bool Window::glfw_initialized_ = false;

Window::Window(const WindowConfig& config)
    : window_(nullptr)
    , event_dispatcher_(&wip::utils::event::global_dispatcher())
    , last_cursor_x_(0.0)
    , last_cursor_y_(0.0)
    , first_cursor_move_(true)
{
    initialize_glfw();
    
    // Set window hints based on configuration
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, config.visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, config.decorated ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, config.focused ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, config.auto_iconify ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, config.floating ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, config.maximized ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, config.samples);
    
    // OpenGL context configuration
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.context_version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.context_version_minor);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, config.forward_compatible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, config.profile);
    
    // Create the window
    window_ = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
    if (!window_) {
        cleanup_glfw();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    // Store this instance in the window user pointer
    glfwSetWindowUserPointer(window_, this);
    
    // Set up callbacks
    setup_callbacks();
    
    // Initialize cursor position
    glfwGetCursorPos(window_, &last_cursor_x_, &last_cursor_y_);
    
    glfw_window_count_++;
}

Window::Window(int width, int height, const std::string& title)
    : Window(WindowConfig{width, height, title})
{
}

Window::~Window() {
    if (window_) {
        glfwDestroyWindow(window_);
        glfw_window_count_--;
        cleanup_glfw();
    }
}

Window::Window(Window&& other) noexcept
    : window_(other.window_)
    , event_dispatcher_(other.event_dispatcher_)
    , last_cursor_x_(other.last_cursor_x_)
    , last_cursor_y_(other.last_cursor_y_)
    , first_cursor_move_(other.first_cursor_move_)
{
    other.window_ = nullptr;
    
    if (window_) {
        glfwSetWindowUserPointer(window_, this);
    }
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (window_) {
            glfwDestroyWindow(window_);
            glfw_window_count_--;
            cleanup_glfw();
        }
        
        window_ = other.window_;
        event_dispatcher_ = other.event_dispatcher_;
        last_cursor_x_ = other.last_cursor_x_;
        last_cursor_y_ = other.last_cursor_y_;
        first_cursor_move_ = other.first_cursor_move_;
        
        other.window_ = nullptr;
        
        if (window_) {
            glfwSetWindowUserPointer(window_, this);
        }
    }
    return *this;
}

bool Window::should_close() const {
    return window_ && glfwWindowShouldClose(window_);
}

void Window::set_should_close(bool should_close) {
    if (window_) {
        glfwSetWindowShouldClose(window_, should_close ? GLFW_TRUE : GLFW_FALSE);
    }
}

void Window::make_context_current() {
    if (window_) {
        glfwMakeContextCurrent(window_);
    }
}

void Window::swap_buffers() {
    if (window_) {
        glfwSwapBuffers(window_);
    }
}

void Window::poll_events() {
    glfwPollEvents();
}

void Window::wait_events() {
    glfwWaitEvents();
}

void Window::get_size(int& width, int& height) const {
    if (window_) {
        glfwGetWindowSize(window_, &width, &height);
    } else {
        width = height = 0;
    }
}

std::pair<int, int> Window::get_size() const {
    int width, height;
    get_size(width, height);
    return {width, height};
}

void Window::set_size(int width, int height) {
    if (window_) {
        glfwSetWindowSize(window_, width, height);
    }
}

void Window::get_position(int& x, int& y) const {
    if (window_) {
        glfwGetWindowPos(window_, &x, &y);
    } else {
        x = y = 0;
    }
}

std::pair<int, int> Window::get_position() const {
    int x, y;
    get_position(x, y);
    return {x, y};
}

void Window::set_position(int x, int y) {
    if (window_) {
        glfwSetWindowPos(window_, x, y);
    }
}

void Window::get_framebuffer_size(int& width, int& height) const {
    if (window_) {
        glfwGetFramebufferSize(window_, &width, &height);
    } else {
        width = height = 0;
    }
}

std::pair<int, int> Window::get_framebuffer_size() const {
    int width, height;
    get_framebuffer_size(width, height);
    return {width, height};
}

void Window::get_cursor_position(double& x, double& y) const {
    if (window_) {
        glfwGetCursorPos(window_, &x, &y);
    } else {
        x = y = 0.0;
    }
}

std::pair<double, double> Window::get_cursor_position() const {
    double x, y;
    get_cursor_position(x, y);
    return {x, y};
}

void Window::set_cursor_position(double x, double y) {
    if (window_) {
        glfwSetCursorPos(window_, x, y);
        last_cursor_x_ = x;
        last_cursor_y_ = y;
    }
}

void Window::show() {
    if (window_) {
        glfwShowWindow(window_);
    }
}

void Window::hide() {
    if (window_) {
        glfwHideWindow(window_);
    }
}

bool Window::is_visible() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_VISIBLE);
}

void Window::iconify() {
    if (window_) {
        glfwIconifyWindow(window_);
    }
}

void Window::restore() {
    if (window_) {
        glfwRestoreWindow(window_);
    }
}

void Window::maximize() {
    if (window_) {
        glfwMaximizeWindow(window_);
    }
}

bool Window::is_iconified() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_ICONIFIED);
}

bool Window::is_maximized() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_MAXIMIZED);
}

void Window::focus() {
    if (window_) {
        glfwFocusWindow(window_);
    }
}

bool Window::is_focused() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_FOCUSED);
}

void Window::set_event_dispatcher(wip::utils::event::EventDispatcher* dispatcher) {
    event_dispatcher_ = dispatcher ? dispatcher : &wip::utils::event::global_dispatcher();
}

wip::utils::event::EventDispatcher* Window::get_event_dispatcher() const {
    return event_dispatcher_;
}

void Window::set_vsync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
}

bool Window::is_glfw_initialized() {
    return glfw_initialized_;
}

std::string Window::get_glfw_version() {
    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    
    std::ostringstream oss;
    oss << major << "." << minor << "." << revision;
    return oss.str();
}

void Window::initialize_glfw() {
    if (!glfw_initialized_) {
        glfwSetErrorCallback(glfw_error_callback);
        
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        
        glfw_initialized_ = true;
    }
}

void Window::cleanup_glfw() {
    if (glfw_initialized_ && glfw_window_count_ == 0) {
        glfwTerminate();
        glfw_initialized_ = false;
    }
}

void Window::setup_callbacks() {
    glfwSetWindowCloseCallback(window_, window_close_callback);
    glfwSetWindowSizeCallback(window_, window_size_callback);
    glfwSetWindowPosCallback(window_, window_pos_callback);
    glfwSetWindowFocusCallback(window_, window_focus_callback);
    glfwSetWindowIconifyCallback(window_, window_iconify_callback);
    glfwSetWindowMaximizeCallback(window_, window_maximize_callback);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
    
    glfwSetKeyCallback(window_, key_callback);
    glfwSetCharCallback(window_, char_callback);
    glfwSetMouseButtonCallback(window_, mouse_button_callback);
    glfwSetCursorPosCallback(window_, cursor_pos_callback);
    glfwSetScrollCallback(window_, scroll_callback);
}

// GLFW Callback implementations
void Window::window_close_callback(GLFWwindow* window) {
    if (auto* win = get_window_instance(window)) {
        win->event_dispatcher_->dispatch(events::WindowEvent(events::WindowEvent::Type::Close));
    }
}

void Window::window_size_callback(GLFWwindow* window, int width, int height) {
    if (auto* win = get_window_instance(window)) {
        win->event_dispatcher_->dispatch(events::WindowEvent(events::WindowEvent::Type::Resize, width, height));
    }
}

void Window::window_pos_callback(GLFWwindow* window, int x, int y) {
    if (auto* win = get_window_instance(window)) {
        win->event_dispatcher_->dispatch(events::WindowEvent(events::WindowEvent::Type::Move, 0, 0, x, y));
    }
}

void Window::window_focus_callback(GLFWwindow* window, int focused) {
    if (auto* win = get_window_instance(window)) {
        auto type = focused ? events::WindowEvent::Type::Focus : events::WindowEvent::Type::Unfocus;
        win->event_dispatcher_->dispatch(events::WindowEvent(type));
    }
}

void Window::window_iconify_callback(GLFWwindow* window, int iconified) {
    if (auto* win = get_window_instance(window)) {
        auto type = iconified ? events::WindowEvent::Type::Iconify : events::WindowEvent::Type::Restore;
        win->event_dispatcher_->dispatch(events::WindowEvent(type));
    }
}

void Window::window_maximize_callback(GLFWwindow* window, int maximized) {
    if (auto* win = get_window_instance(window)) {
        if (maximized) {
            win->event_dispatcher_->dispatch(events::WindowEvent(events::WindowEvent::Type::Maximize));
        }
    }
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (auto* win = get_window_instance(window)) {
        win->event_dispatcher_->dispatch(events::WindowEvent(events::WindowEvent::Type::FramebufferResize, width, height));
    }
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (auto* win = get_window_instance(window)) {
        events::KeyboardEvent::Action event_action;
        switch (action) {
            case GLFW_PRESS:
                event_action = events::KeyboardEvent::Action::Press;
                break;
            case GLFW_RELEASE:
                event_action = events::KeyboardEvent::Action::Release;
                break;
            case GLFW_REPEAT:
                event_action = events::KeyboardEvent::Action::Repeat;
                break;
            default:
                return; // Unknown action
        }
        
        win->event_dispatcher_->dispatch(events::KeyboardEvent(key, scancode, event_action, mods));
    }
}

void Window::char_callback(GLFWwindow* window, unsigned int codepoint) {
    if (auto* win = get_window_instance(window)) {
        win->event_dispatcher_->dispatch(events::CharacterEvent(codepoint));
    }
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (auto* win = get_window_instance(window)) {
        events::MouseButtonEvent::Action event_action;
        switch (action) {
            case GLFW_PRESS:
                event_action = events::MouseButtonEvent::Action::Press;
                break;
            case GLFW_RELEASE:
                event_action = events::MouseButtonEvent::Action::Release;
                break;
            default:
                return; // Unknown action
        }
        
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        win->event_dispatcher_->dispatch(events::MouseButtonEvent(button, event_action, mods, x, y));
    }
}

void Window::cursor_pos_callback(GLFWwindow* window, double x, double y) {
    if (auto* win = get_window_instance(window)) {
        double dx = 0.0, dy = 0.0;
        
        if (!win->first_cursor_move_) {
            dx = x - win->last_cursor_x_;
            dy = y - win->last_cursor_y_;
        } else {
            win->first_cursor_move_ = false;
        }
        
        win->last_cursor_x_ = x;
        win->last_cursor_y_ = y;
        
        win->event_dispatcher_->dispatch(events::MouseMoveEvent(x, y, dx, dy));
    }
}

void Window::scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
    if (auto* win = get_window_instance(window)) {
        win->event_dispatcher_->dispatch(events::MouseScrollEvent(x_offset, y_offset));
    }
}

void Window::glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

Window* Window::get_window_instance(GLFWwindow* glfw_window) {
    return static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
}

} // namespace wip::gui::window