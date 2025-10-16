#include <application.h>
#include <layer.h>
#include <imgui.h>
#include <GL/gl.h>
#include <memory>
#include <iostream>

using namespace wip::gui::application;
using namespace wip::gui::window::events;

// Modern theme configuration
struct ModernTheme {
    ImVec4 main_color = ImVec4(0.15f, 0.16f, 0.21f, 1.00f);      // Dark blue-gray
    ImVec4 secondary_color = ImVec4(0.20f, 0.22f, 0.27f, 1.00f); // Lighter blue-gray
    ImVec4 accent_color = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);    // Bright blue
    ImVec4 text_color = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);      // Light gray
    ImVec4 text_disabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);   // Disabled gray
    ImVec4 background = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);      // Very dark background
};

// Font management
struct FontSystem {
    ImFont* regular_font = nullptr;
    ImFont* medium_font = nullptr;
    ImFont* bold_font = nullptr;
    bool fonts_loaded = false;
    
    void load_fonts() {
        ImGuiIO& io = ImGui::GetIO();
        
        // Configure font loading
        io.Fonts->AddFontDefault(); // Fallback font
        
        // Load Figtree fonts with different weights
        const std::string font_path = "Figtree/static/";
        
        // Try to load Figtree Regular (16px)
        regular_font = io.Fonts->AddFontFromFileTTF((font_path + "Figtree-Regular.ttf").c_str(), 16.0f);
        if (regular_font == nullptr) {
            std::cout << "[FONT] Warning: Could not load Figtree-Regular.ttf, using default font\n";
            regular_font = io.Fonts->Fonts[0]; // Use default font as fallback
        } else {
            std::cout << "[FONT] Loaded Figtree-Regular.ttf successfully\n";
        }
        
        // Try to load Figtree Medium (16px) for headers
        medium_font = io.Fonts->AddFontFromFileTTF((font_path + "Figtree-Medium.ttf").c_str(), 18.0f);
        if (medium_font == nullptr) {
            std::cout << "[FONT] Warning: Could not load Figtree-Medium.ttf, using regular font\n";
            medium_font = regular_font;
        } else {
            std::cout << "[FONT] Loaded Figtree-Medium.ttf successfully\n";
        }
        
        // Try to load Figtree Bold (16px) for emphasis
        bold_font = io.Fonts->AddFontFromFileTTF((font_path + "Figtree-Bold.ttf").c_str(), 16.0f);
        if (bold_font == nullptr) {
            std::cout << "[FONT] Warning: Could not load Figtree-Bold.ttf, using regular font\n";
            bold_font = regular_font;
        } else {
            std::cout << "[FONT] Loaded Figtree-Bold.ttf successfully\n";
        }
        
        // Font atlas will be built automatically by ImGui
        
        // Set Figtree Regular as the default font for all ImGui text
        if (regular_font) {
            io.FontDefault = regular_font;
            std::cout << "[FONT] Set Figtree-Regular as default font\n";
        }
        
        fonts_loaded = true;
        
        std::cout << "[FONT] Font system initialized with Figtree family\n";
    }
};

// Empty layer for you to implement your application logic
class MainAppLayer : public Layer {
private:
    bool first_frame = true;
    ModernTheme current_theme;
    FontSystem font_system;

public:
    MainAppLayer() : Layer("MainApp") {
        // Load fonts first
        font_system.load_fonts();
        
        // Apply modern theme on startup
        apply_modern_theme(current_theme.main_color, current_theme.secondary_color, current_theme.accent_color);
    }

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
    void apply_modern_theme(ImVec4 main_color, ImVec4 secondary_color, ImVec4 accent_color) {
        ImGuiStyle& style = ImGui::GetStyle();
        
        // Window styling
        style.WindowRounding = 6.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;
        
        // Spacing and padding
        style.WindowPadding = ImVec2(12, 12);
        style.FramePadding = ImVec2(8, 6);
        style.ItemSpacing = ImVec2(8, 6);
        style.ItemInnerSpacing = ImVec2(6, 4);
        style.IndentSpacing = 20.0f;
        style.ScrollbarSize = 16.0f;
        style.GrabMinSize = 12.0f;
        
        // Borders
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;
        
        // Color scheme
        ImVec4* colors = style.Colors;
        
        // Window colors
        colors[ImGuiCol_WindowBg] = main_color;
        colors[ImGuiCol_ChildBg] = ImVec4(main_color.x + 0.02f, main_color.y + 0.02f, main_color.z + 0.02f, main_color.w);
        colors[ImGuiCol_PopupBg] = secondary_color;
        colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        
        // Frame colors (buttons, inputs, etc.)
        colors[ImGuiCol_FrameBg] = secondary_color;
        colors[ImGuiCol_FrameBgHovered] = ImVec4(secondary_color.x + 0.1f, secondary_color.y + 0.1f, secondary_color.z + 0.1f, secondary_color.w);
        colors[ImGuiCol_FrameBgActive] = ImVec4(accent_color.x * 0.8f, accent_color.y * 0.8f, accent_color.z * 0.8f, accent_color.w);
        
        // Title bar
        colors[ImGuiCol_TitleBg] = ImVec4(main_color.x - 0.05f, main_color.y - 0.05f, main_color.z - 0.05f, main_color.w);
        colors[ImGuiCol_TitleBgActive] = main_color;
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(main_color.x - 0.1f, main_color.y - 0.1f, main_color.z - 0.1f, main_color.w);
        
        // Menu bar
        colors[ImGuiCol_MenuBarBg] = ImVec4(main_color.x - 0.03f, main_color.y - 0.03f, main_color.z - 0.03f, main_color.w);
        
        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(main_color.x + 0.05f, main_color.y + 0.05f, main_color.z + 0.05f, main_color.w);
        colors[ImGuiCol_ScrollbarGrab] = secondary_color;
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(secondary_color.x + 0.1f, secondary_color.y + 0.1f, secondary_color.z + 0.1f, secondary_color.w);
        colors[ImGuiCol_ScrollbarGrabActive] = accent_color;
        
        // Checkboxes and radio buttons
        colors[ImGuiCol_CheckMark] = accent_color;
        
        // Sliders
        colors[ImGuiCol_SliderGrab] = accent_color;
        colors[ImGuiCol_SliderGrabActive] = ImVec4(accent_color.x * 1.2f, accent_color.y * 1.2f, accent_color.z * 1.2f, accent_color.w);
        
        // Buttons
        colors[ImGuiCol_Button] = secondary_color;
        colors[ImGuiCol_ButtonHovered] = ImVec4(secondary_color.x + 0.1f, secondary_color.y + 0.1f, secondary_color.z + 0.1f, secondary_color.w);
        colors[ImGuiCol_ButtonActive] = accent_color;
        
        // Headers (collapsing headers, selectables, etc.)
        colors[ImGuiCol_Header] = ImVec4(accent_color.x * 0.7f, accent_color.y * 0.7f, accent_color.z * 0.7f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(accent_color.x * 0.8f, accent_color.y * 0.8f, accent_color.z * 0.8f, 0.80f);
        colors[ImGuiCol_HeaderActive] = accent_color;
        
        // Separators
        colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_SeparatorHovered] = accent_color;
        colors[ImGuiCol_SeparatorActive] = ImVec4(accent_color.x * 1.2f, accent_color.y * 1.2f, accent_color.z * 1.2f, accent_color.w);
        
        // Resize grips
        colors[ImGuiCol_ResizeGrip] = ImVec4(accent_color.x * 0.7f, accent_color.y * 0.7f, accent_color.z * 0.7f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(accent_color.x * 0.8f, accent_color.y * 0.8f, accent_color.z * 0.8f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = accent_color;
        
        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(main_color.x + 0.05f, main_color.y + 0.05f, main_color.z + 0.05f, main_color.w);
        colors[ImGuiCol_TabHovered] = ImVec4(accent_color.x * 0.8f, accent_color.y * 0.8f, accent_color.z * 0.8f, 0.80f);
        colors[ImGuiCol_TabSelected] = ImVec4(accent_color.x * 0.9f, accent_color.y * 0.9f, accent_color.z * 0.9f, accent_color.w);
        
        // Docking
        colors[ImGuiCol_DockingPreview] = ImVec4(accent_color.x, accent_color.y, accent_color.z, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        
        // Text colors
        colors[ImGuiCol_Text] = current_theme.text_color;
        colors[ImGuiCol_TextDisabled] = current_theme.text_disabled;
        colors[ImGuiCol_TextSelectedBg] = ImVec4(accent_color.x, accent_color.y, accent_color.z, 0.43f);
        
        // Update current theme colors
        current_theme.main_color = main_color;
        current_theme.secondary_color = secondary_color;
        current_theme.accent_color = accent_color;
    }

    void render_main_content() {
        // Set window position and size for consistent layout
        if (first_frame) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + 19));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.75f, viewport->WorkSize.y * 0.80f - 19));
        }
        
        ImGui::Begin("Main Content");
        
        // Welcome section with medium font
        if (font_system.medium_font) {
            ImGui::PushFont(font_system.medium_font, 0.0f);
            ImGui::Text("🚀 Welcome to your Modern Desktop Application!");
            ImGui::PopFont();
        } else {
            ImGui::Text("🚀 Welcome to your Modern Desktop Application!");
        }
        
        ImGui::Spacing();
        
        // Feature showcase
        if (ImGui::CollapsingHeader("✨ Modern UI Features", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            
            ImGui::Text("🎨 Real-time theme customization");
            ImGui::BulletText("Customizable main, secondary, and accent colors");
            ImGui::BulletText("5 beautiful preset themes included");
            ImGui::BulletText("Modern rounded corners and smooth styling");
            
            ImGui::Spacing();
            ImGui::Text("🔧 Professional UI Components");
            ImGui::BulletText("Responsive layout with consistent proportions");
            ImGui::BulletText("Collapsible sections with icons");
            ImGui::BulletText("Modern button styles with hover effects");
            ImGui::BulletText("Styled input controls and sliders");
            
            ImGui::Separator();
        }
        
        if (ImGui::CollapsingHeader("🏗️ Framework Integration", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            
            ImGui::Text("Framework components available:");
            ImGui::BulletText("Layer-based architecture");
            ImGui::BulletText("Event system integration");
            ImGui::BulletText("ImGui docking support with modern themes");
            ImGui::BulletText("Window management");
            ImGui::BulletText("All WIP utility libraries");
            
            ImGui::Separator();
        }
        
        if (ImGui::CollapsingHeader("💡 Development Guide", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            
            ImGui::Text("Add your application logic in:");
            ImGui::BulletText("on_update() - for logic updates");
            ImGui::BulletText("on_render() - for UI rendering");
            ImGui::BulletText("on_event() - for event handling");
            ImGui::BulletText("apply_modern_theme() - for custom themes");
            
            ImGui::Spacing();
            
            // Interactive example with modern styling
            ImGui::Text("🎯 Interactive Example:");
            static int counter = 0;
            if (ImGui::Button("🔢 Click Counter", ImVec2(150, 0))) {
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("Count: %d", counter);
            
            ImGui::Spacing();
            
            // Progress bar example
            static float progress = 0.0f;
            progress += 0.0005f;
            if (progress >= 1.0f) progress = 0.0f;
            
            ImGui::Text("📊 Progress Example:");
            ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
            
            ImGui::Separator();
        }
        
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
        
        // Theme Customization Section
        if (ImGui::CollapsingHeader("🎨 Theme Customization", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            
            // Font information with samples
            ImGui::Text("🔤 Font: Figtree Family");
            ImGui::BulletText("Regular: %s", font_system.regular_font && font_system.fonts_loaded ? "Loaded ✓" : "Fallback ⚠️");
            ImGui::BulletText("Medium: %s", font_system.medium_font && font_system.fonts_loaded ? "Loaded ✓" : "Fallback ⚠️"); 
            ImGui::BulletText("Bold: %s", font_system.bold_font && font_system.fonts_loaded ? "Loaded ✓" : "Fallback ⚠️");
            
            // Font samples
            ImGui::Spacing();
            ImGui::Text("Font Samples:");
            
            // Regular font sample (default)
            ImGui::Text("Regular: The quick brown fox jumps over the lazy dog");
            
            // Medium font sample
            if (font_system.medium_font) {
                ImGui::PushFont(font_system.medium_font, 0.0f);
                ImGui::Text("Medium: The quick brown fox jumps over the lazy dog");
                ImGui::PopFont();
            }
            
            // Bold font sample
            if (font_system.bold_font) {
                ImGui::PushFont(font_system.bold_font, 0.0f);
                ImGui::Text("Bold: The quick brown fox jumps over the lazy dog");
                ImGui::PopFont();
            }
            
            ImGui::Separator();
            ImGui::Spacing();
            
            // Color pickers for theme colors
            bool theme_changed = false;
            
            ImGui::Text("Main Color (Window Background):");
            if (ImGui::ColorEdit3("##MainColor", (float*)&current_theme.main_color, ImGuiColorEditFlags_NoInputs)) {
                theme_changed = true;
            }
            
            ImGui::Spacing();
            ImGui::Text("Secondary Color (Controls):");
            if (ImGui::ColorEdit3("##SecondaryColor", (float*)&current_theme.secondary_color, ImGuiColorEditFlags_NoInputs)) {
                theme_changed = true;
            }
            
            ImGui::Spacing();
            ImGui::Text("Accent Color (Highlights):");
            if (ImGui::ColorEdit3("##AccentColor", (float*)&current_theme.accent_color, ImGuiColorEditFlags_NoInputs)) {
                theme_changed = true;
            }
            
            // Apply theme changes in real-time
            if (theme_changed) {
                apply_modern_theme(current_theme.main_color, current_theme.secondary_color, current_theme.accent_color);
            }
            
            ImGui::Spacing();
            
            // Preset themes
            if (ImGui::Button("🌙 Dark Blue", ImVec2(-1, 0))) {
                apply_modern_theme(
                    ImVec4(0.15f, 0.16f, 0.21f, 1.00f), // Dark blue-gray
                    ImVec4(0.20f, 0.22f, 0.27f, 1.00f), // Lighter blue-gray
                    ImVec4(0.26f, 0.59f, 0.98f, 1.00f)  // Bright blue
                );
            }
            
            if (ImGui::Button("🌿 Forest Green", ImVec2(-1, 0))) {
                apply_modern_theme(
                    ImVec4(0.12f, 0.18f, 0.15f, 1.00f), // Dark green
                    ImVec4(0.18f, 0.25f, 0.20f, 1.00f), // Medium green
                    ImVec4(0.34f, 0.74f, 0.47f, 1.00f)  // Bright green
                );
            }
            
            if (ImGui::Button("🔥 Ember Orange", ImVec2(-1, 0))) {
                apply_modern_theme(
                    ImVec4(0.18f, 0.13f, 0.12f, 1.00f), // Dark red-brown
                    ImVec4(0.25f, 0.18f, 0.16f, 1.00f), // Medium red-brown
                    ImVec4(0.95f, 0.52f, 0.26f, 1.00f)  // Orange accent
                );
            }
            
            if (ImGui::Button("🌸 Rose Pink", ImVec2(-1, 0))) {
                apply_modern_theme(
                    ImVec4(0.18f, 0.12f, 0.15f, 1.00f), // Dark rose
                    ImVec4(0.25f, 0.17f, 0.20f, 1.00f), // Medium rose
                    ImVec4(0.90f, 0.45f, 0.70f, 1.00f)  // Pink accent
                );
            }
            
            if (ImGui::Button("⚡ Electric Purple", ImVec2(-1, 0))) {
                apply_modern_theme(
                    ImVec4(0.16f, 0.12f, 0.18f, 1.00f), // Dark purple
                    ImVec4(0.22f, 0.17f, 0.25f, 1.00f), // Medium purple
                    ImVec4(0.67f, 0.35f, 0.90f, 1.00f)  // Bright purple
                );
            }
            
            ImGui::Separator();
        }
        
        // Example Controls Section
        if (ImGui::CollapsingHeader("⚙️ Example Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
        
            // Example properties
            static float value = 0.5f;
            ImGui::SliderFloat("Example Value", &value, 0.0f, 1.0f);
            
            static bool checkbox = false;
            ImGui::Checkbox("Example Checkbox", &checkbox);
            
            static char text_buffer[256] = "Hello Modern UI!";
            ImGui::InputText("Text Input", text_buffer, sizeof(text_buffer));
            
            ImGui::Spacing();
            if (ImGui::Button("Primary Button", ImVec2(-1, 0))) {
                std::cout << "Primary button clicked!\n";
            }
        }
        
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
        
        // Console header with bold font
        if (font_system.bold_font) {
            ImGui::PushFont(font_system.bold_font, 0.0f);
            ImGui::Text("📱 Console & Log Output");
            ImGui::PopFont();
        } else {
            ImGui::Text("📱 Console & Log Output");
        }
        ImGui::Separator();
        
        // Log messages with modern styling and colors
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.4f, 1.0f)); // Green for info
        ImGui::Text("[INFO]  Application started successfully");
        ImGui::Text("[INFO]  Modern theme system initialized");
        ImGui::Text("[INFO]  All framework components loaded");
        ImGui::PopStyleColor();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.7f, 0.3f, 1.0f)); // Yellow for warnings
        ImGui::Text("[WARN]  Theme customization is real-time - performance impact possible");
        ImGui::PopStyleColor();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 0.9f, 1.0f)); // Blue for debug
        ImGui::Text("[DEBUG] Current theme: Main(%.2f,%.2f,%.2f) Secondary(%.2f,%.2f,%.2f) Accent(%.2f,%.2f,%.2f)", 
                   current_theme.main_color.x, current_theme.main_color.y, current_theme.main_color.z,
                   current_theme.secondary_color.x, current_theme.secondary_color.y, current_theme.secondary_color.z,
                   current_theme.accent_color.x, current_theme.accent_color.y, current_theme.accent_color.z);
        ImGui::PopStyleColor();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f)); // Gray for system
        ImGui::Text("[SYS]   Ready for user input...");
        ImGui::PopStyleColor();
        
        // Console input area
        ImGui::Spacing();
        ImGui::Separator();
        static char console_input[256] = "";
        if (ImGui::InputText("💬 Command", console_input, sizeof(console_input), ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::cout << "[Console] Command entered: " << console_input << std::endl;
            // Clear input after command
            console_input[0] = '\0';
        }
        
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