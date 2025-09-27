# WIP GUI Window Library

A modern C++ wrapper around GLFW that integrates with the WIP event system, providing strongly-typed window and input events.

## Features

- **Modern C++ Interface**: RAII-based window management with automatic cleanup
- **Event System Integration**: All GLFW callbacks are converted to strongly-typed events
- **Thread-Safe**: All operations are thread-safe using the underlying event system
- **Comprehensive Input Handling**: Keyboard, mouse, character input, and window events
- **High-DPI Support**: Proper framebuffer size handling for high-DPI displays
- **Move Semantics**: Efficient window transfer and resource management

## Quick Start

### Basic Window Creation

```cpp
#include <window.h>
#include <iostream>

using namespace wip::gui::window;

int main() {
    try {
        // Create a window with default settings
        Window window(800, 600, "My Application");
        
        // Subscribe to window close events
        auto close_handle = wip::utils::event::global_dispatcher()
            .subscribe<events::WindowEvent>([&window](const events::WindowEvent& e) {
                if (e.type() == events::WindowEvent::Type::Close) {
                    std::cout << "Window close requested!" << std::endl;
                    window.set_should_close(true);
                }
            });
        
        // Main loop
        while (!window.should_close()) {
            window.poll_events();  // Process all pending events
            
            // Your rendering code here
            
            window.swap_buffers(); // Present the rendered frame
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Advanced Configuration

```cpp
#include <window.h>

using namespace wip::gui::window;

int main() {
    // Configure window creation
    WindowConfig config;
    config.width = 1280;
    config.height = 720;
    config.title = "Advanced Window";
    config.resizable = true;
    config.samples = 4; // 4x MSAA
    config.context_version_major = 4;
    config.context_version_minor = 1;
    
    Window window(config);
    
    // Enable VSync
    window.set_vsync(true);
    
    // Subscribe to various events...
    return 0;
}
```

### Input Event Handling

```cpp
#include <window.h>
#include <event_dispatcher.h>

using namespace wip::gui::window;
using namespace wip::utils::event;

class InputHandler {
public:
    void setup_event_handling() {
        auto& dispatcher = global_dispatcher();
        
        // Keyboard events
        key_handle_ = dispatcher.subscribe<events::KeyboardEvent>(
            [this](const events::KeyboardEvent& e) {
                if (e.action() == events::KeyboardEvent::Action::Press) {
                    if (e.key() == GLFW_KEY_ESCAPE) {
                        std::cout << "ESC pressed - exiting" << std::endl;
                        should_exit_ = true;
                    } else if (e.key() == GLFW_KEY_SPACE && e.is_ctrl()) {
                        std::cout << "Ctrl+Space pressed" << std::endl;
                    }
                }
            }
        );
        
        // Mouse events
        mouse_handle_ = dispatcher.subscribe<events::MouseButtonEvent>(
            [](const events::MouseButtonEvent& e) {
                if (e.action() == events::MouseButtonEvent::Action::Press) {
                    std::cout << "Mouse click at (" << e.x() << ", " << e.y() << ")" << std::endl;
                    
                    if (e.is_left_button()) {
                        std::cout << "Left mouse button" << std::endl;
                    } else if (e.is_right_button()) {
                        std::cout << "Right mouse button" << std::endl;
                    }
                }
            }
        );
        
        // Mouse movement
        move_handle_ = dispatcher.subscribe<events::MouseMoveEvent>(
            [this](const events::MouseMoveEvent& e) {
                last_mouse_x_ = e.x();
                last_mouse_y_ = e.y();
                
                // Only log if mouse moved significantly
                if (std::abs(e.dx()) > 5 || std::abs(e.dy()) > 5) {
                    std::cout << "Mouse moved to (" << e.x() << ", " << e.y() 
                              << ") delta: (" << e.dx() << ", " << e.dy() << ")" << std::endl;
                }
            }
        );
        
        // Window events
        window_handle_ = dispatcher.subscribe<events::WindowEvent>(
            [](const events::WindowEvent& e) {
                switch (e.type()) {
                    case events::WindowEvent::Type::Resize:
                        std::cout << "Window resized to " << e.width() << "x" << e.height() << std::endl;
                        break;
                    case events::WindowEvent::Type::Focus:
                        std::cout << "Window gained focus" << std::endl;
                        break;
                    case events::WindowEvent::Type::Unfocus:
                        std::cout << "Window lost focus" << std::endl;
                        break;
                    case events::WindowEvent::Type::Iconify:
                        std::cout << "Window minimized" << std::endl;
                        break;
                    case events::WindowEvent::Type::Restore:
                        std::cout << "Window restored" << std::endl;
                        break;
                    default:
                        break;
                }
            }
        );
    }
    
    bool should_exit() const { return should_exit_; }

private:
    SubscriptionHandle key_handle_;
    SubscriptionHandle mouse_handle_;
    SubscriptionHandle move_handle_;
    SubscriptionHandle window_handle_;
    
    double last_mouse_x_ = 0.0;
    double last_mouse_y_ = 0.0;
    bool should_exit_ = false;
};
```

### Custom Event Dispatcher

```cpp
#include <window.h>

int main() {
    // Create a custom event dispatcher
    wip::utils::event::EventDispatcher custom_dispatcher;
    
    Window window(800, 600, "Custom Dispatcher Example");
    window.set_event_dispatcher(&custom_dispatcher);
    
    // All window events will now go through custom_dispatcher
    auto handle = custom_dispatcher.subscribe<events::WindowEvent>(
        [](const events::WindowEvent& e) {
            // Handle events with custom logic
        }
    );
    
    // Main loop...
    return 0;
}
```

## Event Types

### Window Events (`events::WindowEvent`)

- `Close` - Window close requested
- `Resize` - Window size changed  
- `Move` - Window position changed
- `Focus/Unfocus` - Window focus state changed
- `Iconify/Restore` - Window minimized/restored
- `Maximize` - Window maximized
- `FramebufferResize` - Framebuffer size changed (for high-DPI)

### Keyboard Events (`events::KeyboardEvent`)

- `Press/Release/Repeat` - Key state changes
- Modifier key detection (`is_shift()`, `is_ctrl()`, `is_alt()`, `is_super()`)
- GLFW key codes and scancodes

### Character Events (`events::CharacterEvent`)

- Text input events for unicode characters
- Separate from key events for proper text handling

### Mouse Events

- `events::MouseButtonEvent` - Mouse button press/release with position
- `events::MouseMoveEvent` - Mouse movement with delta
- `events::MouseScrollEvent` - Mouse wheel/trackpad scrolling

## Window Management

### Basic Operations

```cpp
Window window(800, 600, "Example");

// Size and position
auto [width, height] = window.get_size();
window.set_size(1024, 768);

auto [x, y] = window.get_position(); 
window.set_position(100, 100);

// Visibility and state
window.show();
window.hide();
window.iconify();
window.restore(); 
window.maximize();

// Focus
window.focus();
bool focused = window.is_focused();

// Close control
window.set_should_close(true);
bool should_close = window.should_close();
```

### OpenGL Context

```cpp
Window window(800, 600, "OpenGL Example");

// Make context current for rendering
window.make_context_current();

// Enable/disable VSync
window.set_vsync(true);

// Get framebuffer size for high-DPI displays
auto [fb_width, fb_height] = window.get_framebuffer_size();
glViewport(0, 0, fb_width, fb_height);

// Main rendering loop
while (!window.should_close()) {
    window.poll_events();
    
    // OpenGL rendering calls here
    glClear(GL_COLOR_BUFFER_BIT);
    // ... 
    
    window.swap_buffers();
}
```

## Integration with Existing Code

The Window class is designed to work alongside existing GLFW code:

```cpp
// Access the underlying GLFW handle if needed
GLFWwindow* glfw_handle = window.get_glfw_handle();

// Use with existing GLFW functions
glfwSetWindowUserPointer(glfw_handle, user_data);
```

## Error Handling

```cpp
try {
    Window window(800, 600, "Example");
    // ... use window
} catch (const std::runtime_error& e) {
    std::cerr << "Window creation failed: " << e.what() << std::endl;
    // Handle error (e.g., no display, GLFW init failure)
}
```

## Building

Add to your CMakeLists.txt:

```cmake
find_package(wip_gui_window REQUIRED)
target_link_libraries(your_target PRIVATE wip::gui::window)
```

The library automatically links GLFW and the WIP event system.

## Thread Safety

- All Window operations are thread-safe through the event system
- Event dispatching can occur from any thread
- GLFW context operations should be called from the main thread
- Window creation/destruction should be done on the main thread

## Performance Notes

- `poll_events()` processes all pending events in a single call
- Use `wait_events()` if your application is event-driven and can block
- Event handlers are called synchronously during event processing
- For high-frequency events (mouse movement), consider rate limiting in handlers

This library provides a modern, safe, and efficient way to create and manage windows with integrated event handling for C++ applications.