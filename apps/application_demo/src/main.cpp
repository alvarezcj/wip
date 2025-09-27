#include <application.h>
#include <layer.h>
#include <window.h>
#include <event_dispatcher.h>
#include <common_events.h>

#include <GL/gl.h>
#include <iostream>
#include <chrono>
#include <cmath>

using namespace wip::gui::application;
using namespace wip::gui::window;
using namespace wip::utils::event;

class ApplicationDemo {
public:
    ApplicationDemo() : app_("Application Demo") {
        setup_event_handlers();
    }
    
    void run() {
        std::cout << "Starting Application Demo...\n";
        std::cout << "This demo shows how to use the Application class to manage multiple windows.\n\n";
        
        // Create main window
        auto main_config = WindowConfig{};
        main_config.width = 800;
        main_config.height = 600;
        main_config.title = "Main Window";
        auto main_window_id = app_.create_window(main_config);
        
        std::cout << "Created main window (ID: " << main_window_id << ")\n";
        
        // Create a smaller tool window
        auto tool_window_id = app_.create_window(400, 300, "Tool Window");
        std::cout << "Created tool window (ID: " << tool_window_id << ")\n";
        
        // Position the tool window next to the main window
        if (auto* tool_window = app_.get_window(tool_window_id)) {
            tool_window->set_position(850, 100);
        }
        
        std::cout << "\nActive windows: " << app_.window_count() << "\n";
        std::cout << "Main window: " << (app_.get_main_window() ? "Available" : "Not available") << "\n";
        
        // Display instructions
        std::cout << "\nInstructions:\n";
        std::cout << "- Close any window to remove it from the application\n";
        std::cout << "- Press ESC in any window to quit the application\n";
        std::cout << "- Press '1' to create a new window\n";
        std::cout << "- Press '2' to destroy the tool window\n";
        std::cout << "- Watch the console for event messages\n\n";
        
        // Main application loop
        auto last_time = std::chrono::steady_clock::now();
        
        while (!app_.should_quit()) {
            // Poll events
            app_.poll_events();
            
            // Render all windows
            render_all_windows();
            
            // Print status every few seconds
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_time);
            if (elapsed.count() >= 3) {
                std::cout << "Status: " << app_.window_count() << " windows active\n";
                last_time = current_time;
            }
        }
        
        std::cout << "\nApplication shutting down...\n";
    }

private:
    Application app_;
    int new_window_counter_ = 0;
    
    void setup_event_handlers() {
        auto* dispatcher = app_.get_event_dispatcher();
        
        // Handle window events
        dispatcher->subscribe<events::WindowEvent>([this](const events::WindowEvent& event) {
            switch (event.type()) {
                case events::WindowEvent::Type::Close:
                    std::cout << "Window close requested\n";
                    break;
                case events::WindowEvent::Type::Resize:
                    std::cout << "Window resized to " << event.width() << "x" << event.height() << "\n";
                    break;
                case events::WindowEvent::Type::Focus:
                    std::cout << "Window gained focus\n";
                    break;
                case events::WindowEvent::Type::Unfocus:
                    std::cout << "Window lost focus\n";
                    break;
                default:
                    break;
            }
        });
        
        // Handle keyboard events
        dispatcher->subscribe<events::KeyboardEvent>([this](const events::KeyboardEvent& event) {
            if (event.action() == events::KeyboardEvent::Action::Press) {
                switch (event.key()) {
                    case GLFW_KEY_ESCAPE:
                        std::cout << "ESC pressed - requesting application quit\n";
                        app_.quit();
                        break;
                    case GLFW_KEY_1:
                        create_new_window();
                        break;
                    case GLFW_KEY_2:
                        destroy_tool_window();
                        break;
                    default:
                        break;
                }
            }
        });
        
        // Handle mouse events
        dispatcher->subscribe<events::MouseButtonEvent>([](const events::MouseButtonEvent& event) {
            if (event.action() == events::MouseButtonEvent::Action::Press) {
                std::cout << "Mouse button " << event.button() << " pressed at (" 
                         << event.x() << ", " << event.y() << ")\n";
            }
        });
    }
    
    void create_new_window() {
        ++new_window_counter_;
        
        auto config = WindowConfig{};
        config.width = 300;
        config.height = 200;
        config.title = "New Window " + std::to_string(new_window_counter_);
        
        auto window_id = app_.create_window(config);
        std::cout << "Created new window (ID: " << window_id << ")\n";
        
        // Position it randomly
        if (auto* window = app_.get_window(window_id)) {
            int x = 100 + (new_window_counter_ * 50) % 400;
            int y = 100 + (new_window_counter_ * 30) % 300;
            window->set_position(x, y);
        }
    }
    
    void destroy_tool_window() {
        auto window_ids = app_.get_window_ids();
        for (auto id : window_ids) {
            if (auto* window = app_.get_window(id)) {
                auto [width, height] = window->get_size();
                if (width == 400 && height == 300) { // Find tool window by size
                    app_.destroy_window(id);
                    std::cout << "Destroyed tool window (ID: " << id << ")\n";
                    break;
                }
            }
        }
    }
    
    void render_all_windows() {
        static auto start_time = std::chrono::steady_clock::now();
        
        for (auto window_id : app_.get_window_ids()) {
            if (auto* window = app_.get_window(window_id)) {
                window->make_context_current();
                
                // Simple animated background
                auto current_time = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
                float time = elapsed.count() / 1000.0f;
                
                float r = 0.2f + 0.3f * std::sin(time + window_id);
                float g = 0.3f + 0.3f * std::cos(time + window_id * 0.5f);
                float b = 0.4f + 0.3f * std::sin(time * 0.7f + window_id * 0.3f);
                
                glClearColor(r, g, b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                
                window->swap_buffers();
            }
        }
    }
};

int main() {
    try {
        ApplicationDemo demo;
        demo.run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}