#include <application.h>
#include <layer.h>
#include <window.h>
#include <event_dispatcher.h>
#include <common_events.h>

#include <GL/gl.h>
#include <imgui.h>
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
        frame_count_++;
        total_time_ = timestep.total_time;
    }
    
    bool on_event(const Event& event) override {
        if (auto* kbd_event = dynamic_cast<const events::KeyboardEvent*>(&event)) {
            if (kbd_event->action() == events::KeyboardEvent::Action::Press) {
                if (kbd_event->key() == GLFW_KEY_SPACE) {
                    set_enabled(!is_enabled());
                    std::cout << "[" << get_name() << "] Layer " 
                             << (is_enabled() ? "enabled" : "disabled") << std::endl;
                    return true;
                }
            }
        }
        return false;
    }

    float get_fps() const { return fps_; }
    int get_frame_count() const { return frame_count_; }
    float get_total_time() const { return total_time_; }

private:
    float fps_ = 0.0f;
    int frame_count_ = 0;
    float total_time_ = 0.0f;
};

// ImGui demo layer - now just a regular Layer that calls ImGui functions
class ImGuiDemoLayer : public Layer {
public:
    ImGuiDemoLayer(UILayer* ui_layer) : Layer("ImGuiDemo"), ui_layer_(ui_layer) {}
    
    void on_attach() override {
        std::cout << "[" << get_name() << "] ImGui demo layer attached" << std::endl;
    }
    
    void on_render(Timestep timestep) override {
        // Application handles ImGui frame setup/teardown
        // We just call ImGui functions directly
        render_performance_window();
        render_layer_controls();
        render_demo_window();
        
        if (show_imgui_demo_) {
            ImGui::ShowDemoWindow(&show_imgui_demo_);
        }
    }

private:
    UILayer* ui_layer_;
    bool show_imgui_demo_ = false;
    bool show_performance_window_ = true;
    bool show_layer_controls_ = true;
    bool show_demo_window_ = true;
    float background_color_[3] = {0.2f, 0.3f, 0.4f};
    int counter_ = 0;
    
    void render_performance_window() {
        if (!show_performance_window_) return;
        
        ImGui::Begin("Performance Monitor", &show_performance_window_);
        
        if (ui_layer_) {
            ImGui::Text("FPS: %.1f", ui_layer_->get_fps());
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / ui_layer_->get_fps());
            ImGui::Text("Total Frames: %d", ui_layer_->get_frame_count());
            ImGui::Text("Total Time: %.2f s", ui_layer_->get_total_time());
        }
        
        ImGui::Separator();
        
        static float fps_history[120] = {};
        static int fps_history_offset = 0;
        if (ui_layer_) {
            fps_history[fps_history_offset] = ui_layer_->get_fps();
            fps_history_offset = (fps_history_offset + 1) % 120;
        }
        
        ImGui::PlotLines("FPS", fps_history, 120, fps_history_offset, nullptr, 0.0f, 120.0f, ImVec2(0, 80));
        
        ImGui::End();
    }
    
    void render_layer_controls() {
        if (!show_layer_controls_) return;
        
        ImGui::Begin("Layer Controls", &show_layer_controls_);
        
        ImGui::Text("Layer System Demo");
        ImGui::Separator();
        
        if (ImGui::Button("Toggle UI Layer")) {
            if (ui_layer_) {
                ui_layer_->set_enabled(!ui_layer_->is_enabled());
            }
        }
        
        ImGui::SameLine();
        if (ui_layer_) {
            ImGui::Text("UI Layer: %s", ui_layer_->is_enabled() ? "Enabled" : "Disabled");
        }
        
        if (ImGui::Button("Show ImGui Demo")) {
            show_imgui_demo_ = true;
        }
        
        ImGui::ColorEdit3("Background Color", background_color_);
        
        ImGui::Separator();
        
        ImGui::Checkbox("Show Performance Window", &show_performance_window_);
        ImGui::Checkbox("Show Demo Window", &show_demo_window_);
        
        ImGui::End();
    }
    
    void render_demo_window() {
        if (!show_demo_window_) return;
        
        ImGui::Begin("ImGui Integration Demo", &show_demo_window_);
        
        ImGui::Text("This demonstrates ImGui integration with the layer system!");
        
        ImGui::Separator();
        
        if (ImGui::Button("Click me!")) {
            counter_++;
        }
        ImGui::SameLine();
        ImGui::Text("Counter: %d", counter_);
        
        static float slider_value = 0.5f;
        ImGui::SliderFloat("Demo Slider", &slider_value, 0.0f, 1.0f);
        
        static char text_buffer[256] = "Edit me!";
        ImGui::InputText("Text Input", text_buffer, sizeof(text_buffer));
        
        if (ImGui::CollapsingHeader("More Widgets")) {
            static bool checkbox = false;
            ImGui::Checkbox("Demo Checkbox", &checkbox);
            
            static int radio_selection = 0;
            ImGui::RadioButton("Option 1", &radio_selection, 0); ImGui::SameLine();
            ImGui::RadioButton("Option 2", &radio_selection, 1); ImGui::SameLine();
            ImGui::RadioButton("Option 3", &radio_selection, 2);
            
            static float progress = 0.0f;
            progress += ImGui::GetIO().DeltaTime * 0.1f;
            if (progress > 1.0f) progress = 0.0f;
            ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
        }
        
        ImGui::Separator();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                   1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        ImGui::End();
    }
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
                    std::cout << "[" << get_name() << "] ESC pressed - quitting" << std::endl;
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
        config.name = "ImGui Layer Demo";
        config.default_window_config.width = 1200;
        config.default_window_config.height = 800;
        config.default_window_config.title = "ImGui Integration Demo";
        
        Application app(config);
        app.set_target_fps(60.0f);
        
        // Create window and initialize ImGui
        app.create_window();
        app.initialize_imgui();
        
        // Create layers
        auto background_layer = std::make_unique<BackgroundLayer>();
        auto ui_layer = std::make_unique<UILayer>();
        auto* ui_layer_ptr = ui_layer.get(); // Keep pointer for ImGui layer
        
        // Add layers (order matters!)
        app.add_layer(std::move(background_layer));
        app.add_layer(std::move(ui_layer));
        
        // Add ImGui layer (renders on top) - now just a normal layer
        app.add_layer(std::make_unique<ImGuiDemoLayer>(ui_layer_ptr));
        
        // Add control layer (handles global events)
        app.add_layer(std::make_unique<AppControlLayer>(&app));
        
        std::cout << "Starting ImGui layer demo..." << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "- ESC: Quit application" << std::endl;
        std::cout << "- SPACE: Toggle UI layer" << std::endl;
        std::cout << "- Use ImGui widgets for interactive controls" << std::endl << std::endl;
        
        // Run the application - layers handle everything!
        app.run();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}