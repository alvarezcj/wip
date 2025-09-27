#include <application.h>
#include <layer.h>
#include <imgui.h>
#include <GL/gl.h>
#include <memory>
#include <iostream>

using namespace wip::gui::application;
using namespace wip::gui::window::events;

// Empty layer for you to implement your application logic
class MainAppLayer : public Layer {
private:
    bool first_frame = true;

public:
    MainAppLayer() : Layer("MainApp") {}

    void on_attach() override {
        std::cout << "[MainApp] Layer attached\n";
    }

    void on_detach() override {
        std::cout << "[MainApp] Layer detached\n";
    }

    void on_update(Timestep timestep) override {
        // Update your application logic here
        // timestep.delta_time() gives you frame delta time
        // timestep.total_time() gives you total elapsed time
    }

    void on_render(Timestep timestep) override {
        // Main application window with docking
        static bool show_demo = false;
        static bool show_metrics = false;
        
        // Create a fullscreen dockspace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        bool open = true;
        ImGui::Begin("DockSpace", &open, window_flags);
        ImGui::PopStyleVar(3);

        // Create the dockspace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // Menu bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Ctrl+N")) {
                    // Handle new file
                    std::cout << "New file requested\n";
                }
                if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    // Handle open file
                    std::cout << "Open file requested\n";
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    // Handle save file
                    std::cout << "Save file requested\n";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    // Request application shutdown
                    std::cout << "Exit requested\n";
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Demo Window", nullptr, &show_demo);
                ImGui::MenuItem("Metrics Window", nullptr, &show_metrics);
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    std::cout << "About requested\n";
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }

        ImGui::End();

        // Your main application panels go here
        render_main_content();
        render_sidebar();
        render_console();
        
        // Optional demo windows
        if (show_demo) {
            ImGui::ShowDemoWindow(&show_demo);
        }
        if (show_metrics) {
            ImGui::ShowMetricsWindow(&show_metrics);
        }
        
        // Mark first frame as complete after all windows are set up
        if (first_frame) {
            first_frame = false;
        }
    }

    bool on_event(const wip::utils::event::Event& event) override {
        // Handle application-specific events here
        
        // Example: Handle keyboard events
        if (auto* key_event = dynamic_cast<const KeyboardEvent*>(&event)) {
            if (key_event->action() == KeyboardEvent::Action::Press) {
                switch (key_event->key()) {
                    case GLFW_KEY_F1:
                        std::cout << "[MainApp] F1 pressed - Help\n";
                        return true; // Consume event
                    
                    case GLFW_KEY_F11:
                        std::cout << "[MainApp] F11 pressed - Toggle fullscreen\n";
                        // You can implement fullscreen toggle here
                        return true;
                }
            }
        }
        
        return false; // Don't consume event by default
    }

private:
    void render_main_content() {
        // Set window position and size for consistent layout
        if (first_frame) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + 19));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.75f, viewport->WorkSize.y * 0.80f - 19));
        }
        
        ImGui::Begin("Main Content");
        
        ImGui::Text("Welcome to your desktop application!");
        ImGui::Separator();
        
        ImGui::Text("Framework components available:");
        ImGui::BulletText("Layer-based architecture");
        ImGui::BulletText("Event system integration");
        ImGui::BulletText("ImGui docking support");
        ImGui::BulletText("Window management");
        ImGui::BulletText("All WIP utility libraries");
        
        ImGui::Separator();
        ImGui::Text("Add your application logic in:");
        ImGui::BulletText("on_update() - for logic updates");
        ImGui::BulletText("on_render() - for UI rendering");
        ImGui::BulletText("on_event() - for event handling");
        
        // Example: Simple counter
        static int counter = 0;
        if (ImGui::Button("Click Me!")) {
            counter++;
        }
        ImGui::SameLine();
        ImGui::Text("Counter: %d", counter);
        
        ImGui::End();
    }
    
    void render_sidebar() {
        // Set window position and size for consistent layout
        if (first_frame) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.75f, viewport->WorkPos.y + 19));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.25f, viewport->WorkSize.y - 19));
        }
        
        ImGui::Begin("Properties");
        
        ImGui::Text("Properties Panel");
        ImGui::Separator();
        
        // Example properties
        static float value = 0.5f;
        ImGui::SliderFloat("Example Value", &value, 0.0f, 1.0f);
        
        static bool checkbox = false;
        ImGui::Checkbox("Example Checkbox", &checkbox);
        
        static char text_buffer[256] = "Hello World";
        ImGui::InputText("Text Input", text_buffer, sizeof(text_buffer));
        
        ImGui::End();
    }
    
    void render_console() {
        // Set window position and size for consistent layout
        if (first_frame) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y * 0.80f));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.75f, viewport->WorkSize.y * 0.20f));
        }
        
        ImGui::Begin("Console");
        
        ImGui::Text("Console/Log Panel");
        ImGui::Separator();
        
        // Example log messages
        ImGui::Text("[INFO] Application started successfully");
        ImGui::Text("[INFO] All systems initialized");
        ImGui::Text("[INFO] Ready for user input");
        
        ImGui::End();
    }
};

// Application configuration
struct AppConfig {
    std::string title = "Desktop Application";
    int width = 1200;
    int height = 800;
    float target_fps = 60.0f;
    bool enable_vsync = true;
};

int main() {
    try {
        // Create application configuration
        AppConfig config;
        
        // Create application
        Application app(config.title);
        app.set_target_fps(config.target_fps);
        
        // Create main window
        auto window_id = app.create_window(config.width, config.height, config.title);
        
        // Initialize ImGui with docking support
        app.initialize_imgui();
        
        // Add your application layer
        app.add_layer(std::make_unique<MainAppLayer>());
        
        std::cout << "Starting desktop application...\n";
        std::cout << "Controls:\n";
        std::cout << "  ESC - Quit application\n";
        std::cout << "  F1 - Help\n";
        std::cout << "  F11 - Toggle fullscreen\n";
        std::cout << "  Mouse - Drag to dock windows\n";
        
        // Run the application
        app.run();
        
        std::cout << "Desktop application shut down successfully.\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return 1;
    }
}