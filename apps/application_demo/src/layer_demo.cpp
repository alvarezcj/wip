#include <application.h>
#include <layer.h>
#include <window.h>
#include <event_dispatcher.h>
#include <common_events.h>

#include <GL/gl.h>
#include <iostream>
#include <cmath>

using namespace wip::gui::application;
using namespace wip::gui::window;
using namespace wip::utils::event;

// Background rendering layer
class BackgroundLayer : public Layer {
public:
    BackgroundLayer() : Layer("Background") {}
    
    void on_attach() override {
        std::cout << "[" << get_name() << "] Layer attached" << std::endl;
    }
    
    void on_render(Timestep timestep) override {
        // Simple animated background
        time_ += timestep.delta_time;
        
        float r = 0.2f + 0.1f * std::sin(time_);
        float g = 0.3f + 0.1f * std::cos(time_ * 0.7f);
        float b = 0.4f + 0.1f * std::sin(time_ * 0.5f);
        
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

private:
    float time_ = 0.0f;
};

// UI layer for handling input
class UILayer : public Layer {
public:
    UILayer() : Layer("UI") {}
    
    void on_attach() override {
        std::cout << "[" << get_name() << "] Layer attached" << std::endl;
        std::cout << "Controls: ESC to quit, SPACE to toggle UI layer" << std::endl << std::endl;
    }
    
    void on_update(Timestep timestep) override {
        fps_ = 1.0f / timestep.delta_time;
        
        // Print FPS every 2 seconds
        if (timestep.total_time - last_fps_time_ >= 2.0f) {
            std::cout << "FPS: " << (int)fps_ << std::endl;
            last_fps_time_ = timestep.total_time;
        }
    }
    
    bool on_event(const Event& event) override {
        if (auto* kbd_event = dynamic_cast<const events::KeyboardEvent*>(&event)) {
            if (kbd_event->action() == events::KeyboardEvent::Action::Press) {
                if (kbd_event->key() == GLFW_KEY_SPACE) {
                    set_enabled(!is_enabled());
                    std::cout << "[" << get_name() << "] Layer " 
                             << (is_enabled() ? "enabled" : "disabled") << "\\n";
                    return true;
                }
            }
        }
        return false;
    }

private:
    float fps_ = 0.0f;
    float last_fps_time_ = 0.0f;
};

// Application control layer
class AppControlLayer : public Layer {
public:
    AppControlLayer(Application* app) : Layer("AppControl"), app_(app) {}
    
    void on_attach() override {
        std::cout << "[" << get_name() << "] Layer attached" << std::endl;
    }
    
    bool on_event(const Event& event) override {
        if (auto* kbd_event = dynamic_cast<const events::KeyboardEvent*>(&event)) {
            if (kbd_event->action() == events::KeyboardEvent::Action::Press) {
                if (kbd_event->key() == GLFW_KEY_ESCAPE) {
                    std::cout << "[" << get_name() << "] ESC pressed - quitting\\n";
                    app_->quit();
                    return true;
                }
            }
        }
        return false;
    }

private:
    Application* app_;
};

int main() {
    try {
        // Create application with layer-based architecture
        ApplicationConfig config;
        config.name = "Layer-Based Demo";
        config.default_window_config.width = 800;
        config.default_window_config.height = 600;
        config.default_window_config.title = "Layer Demo";
        
        Application app(config);
        app.set_target_fps(60.0f);
        
        // Create window
        app.create_window();
        
        // Add layers (order matters!)
        app.add_layer(std::make_unique<BackgroundLayer>());
        app.add_layer(std::make_unique<UILayer>());
        app.add_layer(std::make_unique<AppControlLayer>(&app));

        std::cout << "Starting layer-based application..." << std::endl;

        // Run the application - layers handle everything!
        app.run();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}