#include <window.h>
#include <event_dispatcher.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

// OpenGL headers
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

using namespace wip::gui::window;
using namespace wip::gui::window::events;
using namespace wip::utils::event;

class WindowDemo {
public:
    WindowDemo() : running_(true), frame_count_(0) {}
    
    int run() {
        try {
            setup_window();
            setup_event_handlers();
            main_loop();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        
        return 0;
    }

private:
    void setup_window() {
        WindowConfig config;
        config.width = 800;
        config.height = 600;
        config.title = "WIP Window Demo - Event System Integration";
        config.resizable = true;
        config.samples = 4; // Enable 4x MSAA
        
        window_ = std::make_unique<Window>(config);
        window_->make_context_current();
        window_->set_vsync(true);
        
        std::cout << "Window created successfully!" << std::endl;
        std::cout << "GLFW Version: " << Window::get_glfw_version() << std::endl;
        
        auto [width, height] = window_->get_framebuffer_size();
        std::cout << "Framebuffer size: " << width << "x" << height << std::endl;
        
        // Set up basic OpenGL state
        glViewport(0, 0, width, height);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    }
    
    void setup_event_handlers() {
        auto& dispatcher = global_dispatcher();
        
        // Window events
        window_handle_ = dispatcher.subscribe<WindowEvent>([this](const WindowEvent& e) {
            switch (e.type()) {
                case WindowEvent::Type::Close:
                    std::cout << "[WINDOW] Close requested" << std::endl;
                    running_ = false;
                    break;
                    
                case WindowEvent::Type::Resize:
                    std::cout << "[WINDOW] Resized to " << e.width() << "x" << e.height() << std::endl;
                    glViewport(0, 0, e.width(), e.height());
                    break;
                    
                case WindowEvent::Type::FramebufferResize:
                    std::cout << "[WINDOW] Framebuffer resized to " << e.width() << "x" << e.height() << std::endl;
                    glViewport(0, 0, e.width(), e.height());
                    break;
                    
                case WindowEvent::Type::Focus:
                    std::cout << "[WINDOW] Gained focus" << std::endl;
                    break;
                    
                case WindowEvent::Type::Unfocus:
                    std::cout << "[WINDOW] Lost focus" << std::endl;
                    break;
                    
                case WindowEvent::Type::Iconify:
                    std::cout << "[WINDOW] Minimized" << std::endl;
                    break;
                    
                case WindowEvent::Type::Restore:
                    std::cout << "[WINDOW] Restored" << std::endl;
                    break;
                    
                case WindowEvent::Type::Maximize:
                    std::cout << "[WINDOW] Maximized" << std::endl;
                    break;
                    
                case WindowEvent::Type::Move:
                    std::cout << "[WINDOW] Moved to (" << e.x() << ", " << e.y() << ")" << std::endl;
                    break;
            }
        });
        
        // Keyboard events
        key_handle_ = dispatcher.subscribe<KeyboardEvent>([this](const KeyboardEvent& e) {
            if (e.action() == KeyboardEvent::Action::Press) {
                std::cout << "[KEYBOARD] Key pressed: " << e.key();
                
                if (e.is_shift()) std::cout << " +SHIFT";
                if (e.is_ctrl()) std::cout << " +CTRL";
                if (e.is_alt()) std::cout << " +ALT";
                if (e.is_super()) std::cout << " +SUPER";
                
                std::cout << std::endl;
                
                // Handle special keys
                switch (e.key()) {
                    case GLFW_KEY_ESCAPE:
                        std::cout << "[KEYBOARD] ESC pressed - exiting" << std::endl;
                        running_ = false;
                        break;
                        
                    case GLFW_KEY_SPACE:
                        std::cout << "[KEYBOARD] Space pressed - toggling fullscreen" << std::endl;
                        toggle_fullscreen();
                        break;
                        
                    case GLFW_KEY_F11:
                        std::cout << "[KEYBOARD] F11 pressed - toggling fullscreen" << std::endl;
                        toggle_fullscreen();
                        break;
                        
                    case GLFW_KEY_M:
                        if (e.is_ctrl()) {
                            std::cout << "[KEYBOARD] Ctrl+M pressed - minimizing" << std::endl;
                            window_->iconify();
                        }
                        break;
                        
                    case GLFW_KEY_ENTER:
                        if (e.is_alt()) {
                            std::cout << "[KEYBOARD] Alt+Enter pressed - toggling fullscreen" << std::endl;
                            toggle_fullscreen();
                        }
                        break;
                }
            }
        });
        
        // Character input events (for text input)
        char_handle_ = dispatcher.subscribe<CharacterEvent>([](const CharacterEvent& e) {
            char c = e.as_char();
            if (c >= 32 && c < 127) { // Printable ASCII
                std::cout << "[CHAR] Character input: '" << c << "' (codepoint: " << e.codepoint() << ")" << std::endl;
            }
        });
        
        // Mouse button events
        mouse_button_handle_ = dispatcher.subscribe<MouseButtonEvent>([](const MouseButtonEvent& e) {
            std::string button_name = "Unknown";
            if (e.is_left_button()) button_name = "Left";
            else if (e.is_right_button()) button_name = "Right";
            else if (e.is_middle_button()) button_name = "Middle";
            else button_name = "Button" + std::to_string(e.button());
            
            std::string action = (e.action() == MouseButtonEvent::Action::Press) ? "pressed" : "released";
            
            std::cout << "[MOUSE] " << button_name << " button " << action 
                      << " at (" << e.x() << ", " << e.y() << ")" << std::endl;
        });
        
        // Mouse movement events
        mouse_move_handle_ = dispatcher.subscribe<MouseMoveEvent>([this](const MouseMoveEvent& e) {
            mouse_x_ = e.x();
            mouse_y_ = e.y();
            
            // Only log significant movements to avoid spam
            if (std::abs(e.dx()) > 10 || std::abs(e.dy()) > 10) {
                std::cout << "[MOUSE] Moved to (" << e.x() << ", " << e.y() 
                          << ") delta: (" << e.dx() << ", " << e.dy() << ")" << std::endl;
            }
        });
        
        // Mouse scroll events
        scroll_handle_ = dispatcher.subscribe<MouseScrollEvent>([](const MouseScrollEvent& e) {
            std::cout << "[SCROLL] Offset: (" << e.x_offset() << ", " << e.y_offset() << ")" << std::endl;
        });
        
        std::cout << "Event handlers set up successfully!" << std::endl;
        std::cout << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  ESC          - Exit application" << std::endl;
        std::cout << "  SPACE/F11    - Toggle fullscreen" << std::endl;
        std::cout << "  Alt+Enter    - Toggle fullscreen" << std::endl;
        std::cout << "  Ctrl+M       - Minimize window" << std::endl;
        std::cout << "  Mouse        - Move cursor and click buttons" << std::endl;
        std::cout << "  Scroll wheel - Scroll events" << std::endl;
        std::cout << "  Type keys    - Character and keyboard events" << std::endl;
        std::cout << std::endl;
    }
    
    void main_loop() {
        auto last_fps_time = std::chrono::steady_clock::now();
        
        while (running_ && !window_->should_close()) {
            // Process all pending events
            window_->poll_events();
            
            // Simple OpenGL rendering
            render();
            
            // Swap buffers
            window_->swap_buffers();
            
            frame_count_++;
            
            // Print FPS every second
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_fps_time);
            if (duration.count() >= 1) {
                std::cout << "[FPS] " << frame_count_ << " frames in " << duration.count() << " second(s)" << std::endl;
                frame_count_ = 0;
                last_fps_time = now;
            }
        }
        
        std::cout << std::endl << "Exiting main loop..." << std::endl;
    }
    
    void render() {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Simple color animation based on time
        auto now = std::chrono::steady_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        float time = millis / 1000.0f;
        
        float r = (std::sin(time * 0.5f) + 1.0f) * 0.5f;
        float g = (std::sin(time * 0.7f) + 1.0f) * 0.5f;
        float b = (std::sin(time * 0.3f) + 1.0f) * 0.5f;
        
        glClearColor(r * 0.3f + 0.1f, g * 0.3f + 0.1f, b * 0.3f + 0.1f, 1.0f);
        
        // Draw a simple triangle that follows the mouse
        auto [width, height] = window_->get_size();
        
        // Convert mouse coordinates to OpenGL coordinates (-1 to 1)
        float mouse_gl_x = (mouse_x_ / width) * 2.0f - 1.0f;
        float mouse_gl_y = -(mouse_y_ / height) * 2.0f + 1.0f; // Flip Y axis
        
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f); glVertex2f(mouse_gl_x, mouse_gl_y + 0.1f);
        glColor3f(0.0f, 1.0f, 0.0f); glVertex2f(mouse_gl_x - 0.1f, mouse_gl_y - 0.1f);
        glColor3f(0.0f, 0.0f, 1.0f); glVertex2f(mouse_gl_x + 0.1f, mouse_gl_y - 0.1f);
        glEnd();
    }
    
    void toggle_fullscreen() {
        // Note: This is a simplified fullscreen toggle
        // A full implementation would switch between windowed and fullscreen modes
        if (window_->is_maximized()) {
            window_->restore();
        } else {
            window_->maximize();
        }
    }
    
    std::unique_ptr<Window> window_;
    
    // Event subscription handles
    SubscriptionHandle window_handle_;
    SubscriptionHandle key_handle_;
    SubscriptionHandle char_handle_;
    SubscriptionHandle mouse_button_handle_;
    SubscriptionHandle mouse_move_handle_;
    SubscriptionHandle scroll_handle_;
    
    // Application state
    bool running_;
    int frame_count_;
    double mouse_x_ = 0.0;
    double mouse_y_ = 0.0;
};

int main() {
    std::cout << "WIP GUI Window Library Demo" << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << std::endl;
    
    WindowDemo demo;
    return demo.run();
}