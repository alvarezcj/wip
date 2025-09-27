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

// Docking demo layer
class DockingDemoLayer : public Layer {
public:
    DockingDemoLayer(UILayer* ui_layer) : Layer("DockingDemo"), ui_layer_(ui_layer) {}
    
    void on_attach() override {
        std::cout << "[" << get_name() << "] Docking demo layer attached" << std::endl;
    }
    
    void on_render(Timestep timestep) override {
        // Create a dockspace over the entire viewport
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        } else {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspace_open_, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        // Menu bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Options")) {
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_None) != 0)) {
                    dockspace_flags ^= ImGuiDockNodeFlags_None;
                }
                if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { 
                    dockspace_flags ^= ImGuiDockNodeFlags_NoResize; 
                }
                if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) {
                    dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode;
                }
                if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { 
                    dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; 
                }
                if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { 
                    dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; 
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Windows")) {
                ImGui::MenuItem("Performance", NULL, &show_performance_window_);
                ImGui::MenuItem("Controls", NULL, &show_controls_window_);
                ImGui::MenuItem("Tools", NULL, &show_tools_window_);
                ImGui::MenuItem("Properties", NULL, &show_properties_window_);
                ImGui::MenuItem("Console", NULL, &show_console_window_);
                ImGui::MenuItem("ImGui Demo", NULL, &show_imgui_demo_);
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }

        ImGui::End();

        // Show individual dockable windows
        show_performance_window();
        show_controls_window();
        show_tools_window();
        show_properties_window();
        show_console_window();
        
        if (show_imgui_demo_) {
            ImGui::ShowDemoWindow(&show_imgui_demo_);
        }
    }

private:
    UILayer* ui_layer_;
    bool dockspace_open_ = true;
    bool show_performance_window_ = true;
    bool show_controls_window_ = true;
    bool show_tools_window_ = true;
    bool show_properties_window_ = true;
    bool show_console_window_ = true;
    bool show_imgui_demo_ = false;
    
    void show_performance_window() {
        if (!show_performance_window_) return;
        
        if (ImGui::Begin("Performance Monitor", &show_performance_window_)) {
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
        }
        ImGui::End();
    }
    
    void show_controls_window() {
        if (!show_controls_window_) return;
        
        if (ImGui::Begin("Layer Controls", &show_controls_window_)) {
            ImGui::Text("Docking Demo Controls");
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
            
            static float background_color[3] = {0.2f, 0.3f, 0.4f};
            ImGui::ColorEdit3("Background Color", background_color);
        }
        ImGui::End();
    }
    
    void show_tools_window() {
        if (!show_tools_window_) return;
        
        if (ImGui::Begin("Tools", &show_tools_window_)) {
            ImGui::Text("Application Tools");
            ImGui::Separator();
            
            static int tool_selection = 0;
            ImGui::RadioButton("Selection Tool", &tool_selection, 0);
            ImGui::RadioButton("Move Tool", &tool_selection, 1);
            ImGui::RadioButton("Scale Tool", &tool_selection, 2);
            ImGui::RadioButton("Rotate Tool", &tool_selection, 3);
            
            ImGui::Separator();
            
            static float grid_size = 1.0f;
            ImGui::SliderFloat("Grid Size", &grid_size, 0.1f, 10.0f);
            
            static bool snap_to_grid = true;
            ImGui::Checkbox("Snap to Grid", &snap_to_grid);
        }
        ImGui::End();
    }
    
    void show_properties_window() {
        if (!show_properties_window_) return;
        
        if (ImGui::Begin("Properties", &show_properties_window_)) {
            ImGui::Text("Object Properties");
            ImGui::Separator();
            
            static char object_name[256] = "Selected Object";
            ImGui::InputText("Name", object_name, sizeof(object_name));
            
            static float position[3] = {0.0f, 0.0f, 0.0f};
            ImGui::DragFloat3("Position", position, 0.1f);
            
            static float rotation[3] = {0.0f, 0.0f, 0.0f};
            ImGui::DragFloat3("Rotation", rotation, 1.0f, -360.0f, 360.0f);
            
            static float scale[3] = {1.0f, 1.0f, 1.0f};
            ImGui::DragFloat3("Scale", scale, 0.01f, 0.01f, 10.0f);
        }
        ImGui::End();
    }
    
    void show_console_window() {
        if (!show_console_window_) return;
        
        if (ImGui::Begin("Console", &show_console_window_)) {
            static char console_input[256] = "";
            static std::vector<std::string> console_log = {
                "Application started successfully",
                "Layer system initialized",
                "ImGui docking enabled",
                "Ready for user input..."
            };
            
            // Console output area
            ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
            for (const auto& line : console_log) {
                ImGui::Text("> %s", line.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();
            
            // Console input
            if (ImGui::InputText("Command", console_input, sizeof(console_input), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (strlen(console_input) > 0) {
                    console_log.push_back(std::string("Command: ") + console_input);
                    console_log.push_back("Command executed successfully");
                    console_input[0] = 0;
                }
            }
            ImGui::SetItemDefaultFocus();
        }
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
        // Create application with docking support
        ApplicationConfig config;
        config.name = "ImGui Docking Demo";
        config.default_window_config.width = 1400;
        config.default_window_config.height = 900;
        config.default_window_config.title = "Professional Docking Interface";
        
        Application app(config);
        app.set_target_fps(60.0f);
        
        // Create window and initialize ImGui with docking
        app.create_window();
        app.initialize_imgui();
        
        // Create layers
        auto background_layer = std::make_unique<BackgroundLayer>();
        auto ui_layer = std::make_unique<UILayer>();
        auto* ui_layer_ptr = ui_layer.get();
        
        // Add layers (order matters!)
        app.add_layer(std::move(background_layer));
        app.add_layer(std::move(ui_layer));
        
        // Add docking demo layer
        app.add_layer(std::make_unique<DockingDemoLayer>(ui_layer_ptr));
        
        // Add control layer (handles global events)
        app.add_layer(std::make_unique<AppControlLayer>(&app));
        
        std::cout << "Starting ImGui docking demo..." << std::endl;
        std::cout << "Features:" << std::endl;
        std::cout << "- Professional docking interface" << std::endl;
        std::cout << "- Drag windows to dock them" << std::endl;
        std::cout << "- Multi-viewport support" << std::endl;
        std::cout << "- Drag windows outside main window for separate windows" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "- ESC: Quit application" << std::endl;
        std::cout << "- SPACE: Toggle UI layer" << std::endl << std::endl;
        
        // Run the application
        app.run();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}