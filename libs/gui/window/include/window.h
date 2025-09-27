#pragma once

#include <event_dispatcher.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include <functional>

namespace wip::gui::window {

/**
 * @brief GLFW-based events for window and input handling.
 * 
 * These events integrate with the WIP event system and provide
 * strongly-typed GLFW event handling.
 */
namespace events {

/**
 * @brief Window-related events (resize, close, focus, etc.)
 */
class WindowEvent : public wip::utils::event::Event {
public:
    enum class Type {
        Close,           // Window close requested
        Resize,          // Window resized
        Move,            // Window moved
        Focus,           // Window gained focus
        Unfocus,         // Window lost focus
        Iconify,         // Window minimized/iconified
        Restore,         // Window restored from minimized
        Maximize,        // Window maximized
        FramebufferResize // Framebuffer size changed
    };
    
    WindowEvent(Type type, int width = 0, int height = 0, int x = 0, int y = 0)
        : type_(type), width_(width), height_(height), x_(x), y_(y) {}
    
    Type type() const noexcept { return type_; }
    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    int x() const noexcept { return x_; }
    int y() const noexcept { return y_; }

private:
    Type type_;
    int width_, height_;
    int x_, y_;
};

/**
 * @brief Keyboard input events
 */
class KeyboardEvent : public wip::utils::event::Event {
public:
    enum class Action { Press, Release, Repeat };
    
    KeyboardEvent(int key, int scancode, Action action, int mods)
        : key_(key), scancode_(scancode), action_(action), mods_(mods) {}
    
    int key() const noexcept { return key_; }
    int scancode() const noexcept { return scancode_; }
    Action action() const noexcept { return action_; }
    int mods() const noexcept { return mods_; }
    
    bool is_shift() const noexcept { return mods_ & GLFW_MOD_SHIFT; }
    bool is_ctrl() const noexcept { return mods_ & GLFW_MOD_CONTROL; }
    bool is_alt() const noexcept { return mods_ & GLFW_MOD_ALT; }
    bool is_super() const noexcept { return mods_ & GLFW_MOD_SUPER; }

private:
    int key_;
    int scancode_;
    Action action_;
    int mods_;
};

/**
 * @brief Character input events (for text input)
 */
class CharacterEvent : public wip::utils::event::Event {
public:
    explicit CharacterEvent(unsigned int codepoint)
        : codepoint_(codepoint) {}
    
    unsigned int codepoint() const noexcept { return codepoint_; }
    char as_char() const noexcept { return static_cast<char>(codepoint_); }

private:
    unsigned int codepoint_;
};

/**
 * @brief Mouse button events
 */
class MouseButtonEvent : public wip::utils::event::Event {
public:
    enum class Action { Press, Release };
    
    MouseButtonEvent(int button, Action action, int mods, double x, double y)
        : button_(button), action_(action), mods_(mods), x_(x), y_(y) {}
    
    int button() const noexcept { return button_; }
    Action action() const noexcept { return action_; }
    int mods() const noexcept { return mods_; }
    double x() const noexcept { return x_; }
    double y() const noexcept { return y_; }
    
    bool is_left_button() const noexcept { return button_ == GLFW_MOUSE_BUTTON_LEFT; }
    bool is_right_button() const noexcept { return button_ == GLFW_MOUSE_BUTTON_RIGHT; }
    bool is_middle_button() const noexcept { return button_ == GLFW_MOUSE_BUTTON_MIDDLE; }

private:
    int button_;
    Action action_;
    int mods_;
    double x_, y_;
};

/**
 * @brief Mouse cursor movement events
 */
class MouseMoveEvent : public wip::utils::event::Event {
public:
    MouseMoveEvent(double x, double y, double dx, double dy)
        : x_(x), y_(y), dx_(dx), dy_(dy) {}
    
    double x() const noexcept { return x_; }
    double y() const noexcept { return y_; }
    double dx() const noexcept { return dx_; }  // Change in x
    double dy() const noexcept { return dy_; }  // Change in y

private:
    double x_, y_;
    double dx_, dy_;
};

/**
 * @brief Mouse scroll events
 */
class MouseScrollEvent : public wip::utils::event::Event {
public:
    MouseScrollEvent(double x_offset, double y_offset)
        : x_offset_(x_offset), y_offset_(y_offset) {}
    
    double x_offset() const noexcept { return x_offset_; }
    double y_offset() const noexcept { return y_offset_; }

private:
    double x_offset_, y_offset_;
};

} // namespace events

/**
 * @brief Window creation and management configuration
 */
struct WindowConfig {
    int width = 800;
    int height = 600;
    std::string title = "WIP Window";
    bool resizable = true;
    bool visible = true;
    bool decorated = true;
    bool focused = true;
    bool auto_iconify = true;
    bool floating = false;
    bool maximized = false;
    int samples = 0; // MSAA samples (0 = disabled)
    
    // OpenGL context configuration
    int context_version_major = 3;
    int context_version_minor = 3;
    bool forward_compatible = true;
    int profile = GLFW_OPENGL_CORE_PROFILE;
};

/**
 * @brief RAII wrapper around GLFW window functionality
 * 
 * The Window class provides a modern C++ interface to GLFW windows
 * with automatic initialization/cleanup and event system integration.
 */
class Window {
public:
    /**
     * @brief Create a window with the specified configuration
     */
    explicit Window(const WindowConfig& config = WindowConfig{});
    
    /**
     * @brief Create a window with simple parameters
     */
    Window(int width, int height, const std::string& title = "WIP Window");
    
    /**
     * @brief Destructor automatically cleans up GLFW resources
     */
    ~Window();
    
    // Non-copyable but movable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;
    
    /**
     * @brief Check if the window should close
     */
    bool should_close() const;
    
    /**
     * @brief Set whether the window should close
     */
    void set_should_close(bool should_close);
    
    /**
     * @brief Make this window's OpenGL context current
     */
    void make_context_current();
    
    /**
     * @brief Swap front and back buffers
     */
    void swap_buffers();
    
    /**
     * @brief Process events and dispatch them through the event system
     * This should be called regularly in the main loop
     */
    void poll_events();
    
    /**
     * @brief Wait for events (blocking)
     */
    void wait_events();
    
    /**
     * @brief Get window size
     */
    void get_size(int& width, int& height) const;
    std::pair<int, int> get_size() const;
    
    /**
     * @brief Set window size
     */
    void set_size(int width, int height);
    
    /**
     * @brief Get window position
     */
    void get_position(int& x, int& y) const;
    std::pair<int, int> get_position() const;
    
    /**
     * @brief Set window position
     */
    void set_position(int x, int y);
    
    /**
     * @brief Get framebuffer size (for high-DPI displays)
     */
    void get_framebuffer_size(int& width, int& height) const;
    std::pair<int, int> get_framebuffer_size() const;
    
    /**
     * @brief Get cursor position relative to window
     */
    void get_cursor_position(double& x, double& y) const;
    std::pair<double, double> get_cursor_position() const;
    
    /**
     * @brief Set cursor position
     */
    void set_cursor_position(double x, double y);
    
    /**
     * @brief Window visibility control
     */
    void show();
    void hide();
    bool is_visible() const;
    
    /**
     * @brief Window state control
     */
    void iconify();
    void restore();
    void maximize();
    bool is_iconified() const;
    bool is_maximized() const;
    
    /**
     * @brief Focus control
     */
    void focus();
    bool is_focused() const;
    
    /**
     * @brief Get the underlying GLFW window handle (use with caution)
     */
    GLFWwindow* get_glfw_handle() const noexcept { return window_; }
    
    /**
     * @brief Set the event dispatcher for this window
     * By default uses the global dispatcher
     */
    void set_event_dispatcher(wip::utils::event::EventDispatcher* dispatcher);
    
    /**
     * @brief Get the event dispatcher used by this window
     */
    wip::utils::event::EventDispatcher* get_event_dispatcher() const;
    
    /**
     * @brief Enable/disable VSync
     */
    void set_vsync(bool enabled);
    
    /**
     * @brief Check if GLFW is initialized
     */
    static bool is_glfw_initialized();
    
    /**
     * @brief Get GLFW version information
     */
    static std::string get_glfw_version();

private:
    GLFWwindow* window_;
    wip::utils::event::EventDispatcher* event_dispatcher_;
    
    // Previous cursor position for calculating deltas
    double last_cursor_x_ = 0.0;
    double last_cursor_y_ = 0.0;
    bool first_cursor_move_ = true;
    
    // Static GLFW management
    static int glfw_window_count_;
    static bool glfw_initialized_;
    
    void initialize_glfw();
    void cleanup_glfw();
    void setup_callbacks();
    
    // GLFW callback functions
    static void window_close_callback(GLFWwindow* window);
    static void window_size_callback(GLFWwindow* window, int width, int height);
    static void window_pos_callback(GLFWwindow* window, int x, int y);
    static void window_focus_callback(GLFWwindow* window, int focused);
    static void window_iconify_callback(GLFWwindow* window, int iconified);
    static void window_maximize_callback(GLFWwindow* window, int maximized);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void char_callback(GLFWwindow* window, unsigned int codepoint);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double x, double y);
    static void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);
    
    static void glfw_error_callback(int error, const char* description);
    
    // Helper to get Window instance from GLFW window
    static Window* get_window_instance(GLFWwindow* glfw_window);
};

} // namespace wip::gui::window