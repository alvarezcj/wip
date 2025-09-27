#include <application.h>
#include <layer.h>
#include <imgui.h>
#include <GL/gl.h>
#include <iostream>

using namespace wip::gui::application;

// Simple background layer
class BackgroundLayer : public Layer {
public:
    BackgroundLayer() : Layer("Background") {}
    
    void on_render(Timestep timestep) override {
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
};

// Simple ImGui layer - just inherit from Layer and call ImGui functions!
class SimpleUILayer : public Layer {
public:
    SimpleUILayer() : Layer("SimpleUI") {}
    
    void on_render(Timestep timestep) override {
        // Application handles ImGui frame setup - we just call ImGui functions
        ImGui::Begin("Hello ImGui!");
        ImGui::Text("This is so much simpler!");
        
        if (ImGui::Button("Click me!")) {
            counter_++;
        }
        ImGui::Text("Counter: %d", counter_);
        
        static float value = 0.5f;
        ImGui::SliderFloat("Slider", &value, 0.0f, 1.0f);
        
        ImGui::End();
    }
    
private:
    int counter_ = 0;
};

// Control layer for quitting
class ControlLayer : public Layer {
public:
    ControlLayer(Application* app) : Layer("Control"), app_(app) {}
    
    bool on_event(const wip::utils::event::Event& event) override {
        // Handle ESC to quit (would need to cast event for key checking)
        // For simplicity, just showing the pattern
        return false;
    }
    
private:
    Application* app_;
};

int main() {
    try {
        // Create application
        Application app("Simple ImGui Demo");
        
        // Create window and initialize ImGui - that's it!
        app.create_window();
        app.initialize_imgui();
        
        // Add layers - ImGui layers are just normal layers now!
        app.add_layer(std::make_unique<BackgroundLayer>());
        app.add_layer(std::make_unique<SimpleUILayer>());
        app.add_layer(std::make_unique<ControlLayer>(&app));
        
        // Run - everything is handled automatically
        app.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}