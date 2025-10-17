#include <application.h>
#include <layer.h>
#include <imgui.h>
#include <GL/gl.h>
#include <process.h>
#include <widgets.h>
#include "cppcheck_config_widget.h"
#include "log_window_panel.h"
#include "analysis_result_panel.h"
#include "analysis_result.h"
#include "project_manager.h"
#include <nfd.h>
#include <memory>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstring>

using namespace wip::gui::application;
using namespace wip::gui::window::events;
using namespace wip::utils::process;
using namespace gran_azul::widgets;

// Gran Azul theme configuration
struct GranAzulTheme {
    ImVec4 main_color = ImVec4(0.12f, 0.18f, 0.25f, 1.00f);      // Deep blue-gray (Gran Azul inspired)
    ImVec4 secondary_color = ImVec4(0.18f, 0.24f, 0.32f, 1.00f); // Lighter blue-gray
    ImVec4 accent_color = ImVec4(0.20f, 0.60f, 0.86f, 1.00f);    // Gran Azul blue
    ImVec4 text_color = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);      // Light text
    ImVec4 text_disabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);   // Disabled text
    ImVec4 background = ImVec4(0.08f, 0.12f, 0.18f, 1.00f);      // Very dark blue background
};

// Font management system
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
        const std::string font_path = "/home/jalvarez/wip/Figtree/static/";
        
        // Try to load Figtree Regular (16px)
        regular_font = io.Fonts->AddFontFromFileTTF((font_path + "Figtree-Regular.ttf").c_str(), 16.0f);
        if (regular_font == nullptr) {
            std::cout << "[GRAN_AZUL] Warning: Could not load Figtree-Regular.ttf, using default font\n";
            regular_font = io.Fonts->Fonts[0]; // Use default font as fallback
        } else {
            std::cout << "[GRAN_AZUL] Loaded Figtree-Regular.ttf successfully\n";
        }
        
        // Try to load Figtree Medium (18px) for headers
        medium_font = io.Fonts->AddFontFromFileTTF((font_path + "Figtree-Medium.ttf").c_str(), 18.0f);
        if (medium_font == nullptr) {
            std::cout << "[GRAN_AZUL] Warning: Could not load Figtree-Medium.ttf, using regular font\n";
            medium_font = regular_font;
        } else {
            std::cout << "[GRAN_AZUL] Loaded Figtree-Medium.ttf successfully\n";
        }
        
        // Try to load Figtree Bold (16px) for emphasis
        bold_font = io.Fonts->AddFontFromFileTTF((font_path + "Figtree-Bold.ttf").c_str(), 16.0f);
        if (bold_font == nullptr) {
            std::cout << "[GRAN_AZUL] Warning: Could not load Figtree-Bold.ttf, using regular font\n";
            bold_font = regular_font;
        } else {
            std::cout << "[GRAN_AZUL] Loaded Figtree-Bold.ttf successfully\n";
        }
        
        // Set Figtree Regular as the default font
        if (regular_font) {
            io.FontDefault = regular_font;
            std::cout << "[GRAN_AZUL] Set Figtree-Regular as default font\n";
        }
        
        fonts_loaded = true;
        std::cout << "[GRAN_AZUL] Font system initialized\n";
    }
};

// Main application layer for Gran Azul
class GranAzulMainLayer : public Layer {
private:
    bool first_frame = true;
    GranAzulTheme current_theme;
    FontSystem font_system;
    ProcessExecutor process_executor;
    
    // Widgets
    std::unique_ptr<CppcheckConfigWidget> cppcheck_widget_;
    std::unique_ptr<LogWindowPanel> log_panel_;
    std::unique_ptr<AnalysisResultPanel> analysis_panel_;
    
    // Project management
    std::unique_ptr<gran_azul::ProjectManager> project_manager_;
    
    // Window state
    bool show_cppcheck_config = false;

public:
    GranAzulMainLayer() : Layer("GranAzul") {
        // Create widgets
        cppcheck_widget_ = std::make_unique<CppcheckConfigWidget>();
        log_panel_ = std::make_unique<LogWindowPanel>();
        analysis_panel_ = std::make_unique<AnalysisResultPanel>("Analysis Results");
        
        // Create project manager
        project_manager_ = std::make_unique<gran_azul::ProjectManager>();
        
        // Setup widget callbacks
        setup_widget_callbacks();
    }

    void on_attach() override {
        std::cout << "[GRAN_AZUL] Application layer attached\n";
        
        // Initialize NFD (nativefiledialog-extended)
        NFD_Init();
        
        // Now it's safe to initialize ImGui-dependent components
        font_system.load_fonts();
        apply_gran_azul_theme();
        
        std::cout << "[GRAN_AZUL] ImGui components initialized\n";
    }

    void on_detach() override {
        std::cout << "[GRAN_AZUL] Application layer detached\n";
        
        // Cleanup NFD (nativefiledialog-extended)
        NFD_Quit();
    }

    void on_update(Timestep timestep) override {
        // Application logic updates will go here
        // For now, just a basic update loop
    }

    void on_render(Timestep timestep) override {
        // Update widgets
        float delta_time = timestep.seconds();
        cppcheck_widget_->update(delta_time);
        log_panel_->update(delta_time);
        analysis_panel_->update(delta_time);
        
        // Main application window with docking support
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
        ImGui::Begin("Gran Azul - Code Quality Analysis", &open, window_flags);
        ImGui::PopStyleVar(3);

        // Create the dockspace
        ImGuiID dockspace_id = ImGui::GetID("GranAzulDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // Menu bar
        render_menu_bar();

        ImGui::End();

        // Main content area
        render_main_content();
        
        // Log window
        if (log_panel_->is_visible()) {
            log_panel_->draw();
        }
        
        // Analysis results window
        if (analysis_panel_->is_visible()) {
            analysis_panel_->draw();
        }
        
        // Cppcheck configuration window
        if (show_cppcheck_config) {
            render_cppcheck_config_window();
        }
        
        // Mark first frame as complete
        if (first_frame) {
            first_frame = false;
        }
    }

    bool on_event(const wip::utils::event::Event& event) override {
        // Handle application-specific events
        if (auto* key_event = dynamic_cast<const KeyboardEvent*>(&event)) {
            if (key_event->action() == KeyboardEvent::Action::Press) {
                switch (key_event->key()) {
                    case GLFW_KEY_F1:
                        std::cout << "[GRAN_AZUL] F1 pressed - Show help\n";
                        return true;
                    
                    case GLFW_KEY_F11:
                        std::cout << "[GRAN_AZUL] F11 pressed - Toggle fullscreen\n";
                        return true;
                }
            }
        }
        
        return false; // Don't consume event by default
    }

private:
    void setup_widget_callbacks() {
        // Setup cppcheck widget callbacks
        cppcheck_widget_->set_analysis_callback([this](const CppcheckConfig& config) {
            run_cppcheck_analysis(config);
        });
        
        cppcheck_widget_->set_version_callback([this]() {
            run_cppcheck_version();
        });
        
        cppcheck_widget_->set_directory_callback([this](const CppcheckConfig& config) {
            create_build_directory(config);
        });
        
        cppcheck_widget_->set_directory_selection_callback([this]() -> std::string {
            return select_directory_dialog();
        });
        
        // Setup analysis result panel callbacks
        analysis_panel_->set_file_open_callback([this](const std::string& file_path, int line, int column) {
            open_file_at_location(file_path, line, column);
        });
    }
    
    void run_cppcheck_analysis(const CppcheckConfig& config) {
        std::cout << "[GRAN_AZUL] Running cppcheck analysis with configuration\n";
        
        if (!ProcessExecutor::command_exists("cppcheck")) {
            ProcessResult error_result{127, "", "cppcheck command not found", std::chrono::milliseconds(0), false};
            log_panel_->add_log_entry("cppcheck analysis", error_result);
            
            // Show error in analysis panel
            AnalysisResult error_analysis_result;
            error_analysis_result.analysis_successful = false;
            error_analysis_result.error_message = "cppcheck command not found";
            analysis_panel_->set_analysis_result(error_analysis_result);
            return;
        }
        
        // Use the widget's command generation
        auto args = cppcheck_widget_->generate_command_args();
        auto result = process_executor.execute("cppcheck", args);
        
        // Create log entry
        std::string command_str = "cppcheck";
        for (const auto& arg : args) {
            command_str += " " + arg;
        }
        log_panel_->add_log_entry(command_str, result);
        
        std::cout << "[GRAN_AZUL] Cppcheck analysis completed with exit code: " << result.exit_code << "\n";
        std::cout << "[GRAN_AZUL] Output saved to: " << config.output_file << "\n";
        
        // Parse and display analysis results
        parse_and_display_analysis_results(config);
    }
    
    void parse_and_display_analysis_results(const CppcheckConfig& config) {
        AnalysisResult analysis_result;
        
        // Set analysis metadata
        analysis_result.source_path = config.source_path;
        
        // Try to parse the analysis file
        bool parse_success = analysis_parser::parse_analysis_file(config.output_file, analysis_result);
        
        if (parse_success || analysis_result.issues.empty()) {
            // Even if no issues found, mark as successful
            analysis_result.analysis_successful = true;
            std::cout << "[GRAN_AZUL] Analysis results parsed successfully: " 
                      << analysis_result.issues.size() << " issues found\n";
        } else {
            std::cout << "[GRAN_AZUL] Failed to parse analysis results from: " << config.output_file << "\n";
            analysis_result.analysis_successful = false;
            analysis_result.error_message = "Failed to parse analysis output file";
        }
        
        // Display results in analysis panel
        analysis_panel_->set_analysis_result(analysis_result);
        
        // Print summary to console
        if (analysis_result.analysis_successful) {
            std::cout << "[GRAN_AZUL] Analysis Summary:\n";
            std::cout << "  - Total issues: " << analysis_result.issues.size() << "\n";
            std::cout << "  - Errors: " << analysis_result.count_by_severity(IssueSeverity::ERROR) << "\n";
            std::cout << "  - Warnings: " << analysis_result.count_by_severity(IssueSeverity::WARNING) << "\n";
            std::cout << "  - Style issues: " << analysis_result.count_by_severity(IssueSeverity::STYLE) << "\n";
            std::cout << "  - Performance issues: " << analysis_result.count_by_severity(IssueSeverity::PERFORMANCE) << "\n";
        }
    }
    
    void open_file_at_location(const std::string& file_path, int line, int column) {
        std::cout << "[GRAN_AZUL] Request to open file: " << file_path 
                  << " at line " << line << ", column " << column << "\n";
        
        // For now, just log the request. In a full IDE, this would:
        // 1. Open the file in an editor
        // 2. Navigate to the specified line/column
        // 3. Highlight the issue location
        
        // We could also try to open the file with the system default editor
        // std::string command = "code --goto " + file_path + ":" + std::to_string(line) + ":" + std::to_string(column);
        // system(command.c_str());
    }
    
    void run_cppcheck_version() {
        std::cout << "[GRAN_AZUL] Running cppcheck --version\n";
        
        if (!ProcessExecutor::command_exists("cppcheck")) {
            ProcessResult error_result{127, "", "cppcheck command not found", std::chrono::milliseconds(0), false};
            log_panel_->add_log_entry("cppcheck --version", error_result);
            return;
        }
        
        std::vector<std::string> args = {"--version"};
        auto result = process_executor.execute("cppcheck", args);
        
        log_panel_->add_log_entry("cppcheck --version", result);
        
        std::cout << "[GRAN_AZUL] cppcheck --version completed with exit code: " << result.exit_code << "\n";
    }
    
    void create_build_directory(const CppcheckConfig& config) {
        // Create build directory
        std::string cmd = "mkdir -p " + std::string(config.build_dir);
        auto result = process_executor.execute(cmd);
        log_panel_->add_log_entry("mkdir -p " + std::string(config.build_dir), result);
    }
    
    void render_cppcheck_config_window() {
        if (first_frame) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.1f, viewport->WorkPos.y + viewport->WorkSize.y * 0.1f));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.8f, viewport->WorkSize.y * 0.8f));
        }
        
        if (ImGui::Begin("Cppcheck Configuration", &show_cppcheck_config, ImGuiWindowFlags_AlwaysAutoResize)) {
            cppcheck_widget_->draw();
            
            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(80, 0))) {
                show_cppcheck_config = false;
            }
        }
        ImGui::End();
    }

    void apply_gran_azul_theme() {
        ImGuiStyle& style = ImGui::GetStyle();
        
        // Modern window styling
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
        
        // Apply Gran Azul color scheme
        ImVec4* colors = style.Colors;
        
        // Window colors
        colors[ImGuiCol_WindowBg] = current_theme.main_color;
        colors[ImGuiCol_ChildBg] = ImVec4(current_theme.main_color.x + 0.02f, current_theme.main_color.y + 0.02f, 
                                         current_theme.main_color.z + 0.02f, current_theme.main_color.w);
        colors[ImGuiCol_PopupBg] = current_theme.secondary_color;
        colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        
        // Frame colors
        colors[ImGuiCol_FrameBg] = current_theme.secondary_color;
        colors[ImGuiCol_FrameBgHovered] = ImVec4(current_theme.secondary_color.x + 0.1f, current_theme.secondary_color.y + 0.1f, 
                                                current_theme.secondary_color.z + 0.1f, current_theme.secondary_color.w);
        colors[ImGuiCol_FrameBgActive] = ImVec4(current_theme.accent_color.x * 0.8f, current_theme.accent_color.y * 0.8f, 
                                              current_theme.accent_color.z * 0.8f, current_theme.accent_color.w);
        
        // Title bar
        colors[ImGuiCol_TitleBg] = ImVec4(current_theme.main_color.x - 0.05f, current_theme.main_color.y - 0.05f, 
                                         current_theme.main_color.z - 0.05f, current_theme.main_color.w);
        colors[ImGuiCol_TitleBgActive] = current_theme.main_color;
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(current_theme.main_color.x - 0.1f, current_theme.main_color.y - 0.1f, 
                                                  current_theme.main_color.z - 0.1f, current_theme.main_color.w);
        
        // Menu bar
        colors[ImGuiCol_MenuBarBg] = ImVec4(current_theme.main_color.x - 0.03f, current_theme.main_color.y - 0.03f, 
                                           current_theme.main_color.z - 0.03f, current_theme.main_color.w);
        
        // Buttons
        colors[ImGuiCol_Button] = current_theme.secondary_color;
        colors[ImGuiCol_ButtonHovered] = ImVec4(current_theme.secondary_color.x + 0.1f, current_theme.secondary_color.y + 0.1f, 
                                               current_theme.secondary_color.z + 0.1f, current_theme.secondary_color.w);
        colors[ImGuiCol_ButtonActive] = current_theme.accent_color;
        
        // Headers
        colors[ImGuiCol_Header] = ImVec4(current_theme.accent_color.x * 0.7f, current_theme.accent_color.y * 0.7f, 
                                        current_theme.accent_color.z * 0.7f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(current_theme.accent_color.x * 0.8f, current_theme.accent_color.y * 0.8f, 
                                               current_theme.accent_color.z * 0.8f, 0.80f);
        colors[ImGuiCol_HeaderActive] = current_theme.accent_color;
        
        // Text colors
        colors[ImGuiCol_Text] = current_theme.text_color;
        colors[ImGuiCol_TextDisabled] = current_theme.text_disabled;
        colors[ImGuiCol_TextSelectedBg] = ImVec4(current_theme.accent_color.x, current_theme.accent_color.y, 
                                                current_theme.accent_color.z, 0.43f);
        
        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(current_theme.main_color.x + 0.05f, current_theme.main_color.y + 0.05f, 
                                     current_theme.main_color.z + 0.05f, current_theme.main_color.w);
        colors[ImGuiCol_TabHovered] = ImVec4(current_theme.accent_color.x * 0.8f, current_theme.accent_color.y * 0.8f, 
                                            current_theme.accent_color.z * 0.8f, 0.80f);
        colors[ImGuiCol_TabSelected] = ImVec4(current_theme.accent_color.x * 0.9f, current_theme.accent_color.y * 0.9f, 
                                             current_theme.accent_color.z * 0.9f, current_theme.accent_color.w);
    }
    
    // Project management methods
    void create_new_project() {
        std::cout << "[GRAN_AZUL] Create new project requested\n";
        
        nfdchar_t *savePath;
        nfdfilteritem_t filterItem[1] = { { "Gran Azul Projects", "granazul" } };
        
        nfdresult_t result = NFD_SaveDialog(&savePath, filterItem, 1, nullptr, "New Project.granazul");
        
        if (result == NFD_OKAY) {
            std::filesystem::path project_path(savePath);
            std::string project_dir = project_path.parent_path().string();
            
            std::cout << "[GRAN_AZUL] Creating project in directory: " << project_dir << "\n";
            
            // Create project with selected directory as root
            if (project_manager_->create_new_project("New Project", project_dir)) {
                update_ui_from_project();
                
                // Save the project with the selected filename
                if (project_manager_->save_project_as(savePath)) {
                    std::cout << "[GRAN_AZUL] New project created and saved: " << savePath << "\n";
                } else {
                    std::cout << "[GRAN_AZUL] Project created but failed to save\n";
                }
            } else {
                std::cout << "[GRAN_AZUL] Failed to create project\n";
            }
            
            // Remember to free the path memory!
            NFD_FreePath(savePath);
            
        } else if (result == NFD_CANCEL) {
            std::cout << "[GRAN_AZUL] Project creation cancelled\n";
        } else {
            std::cout << "[GRAN_AZUL] Project creation error: " << NFD_GetError() << "\n";
        }
    }
    
    void open_project() {
        std::cout << "[GRAN_AZUL] Open project requested\n";
        
        nfdchar_t *outPath;
        nfdfilteritem_t filterItem[1] = { { "Gran Azul Projects", "granazul" } };
        
        nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, nullptr);
        
        if (result == NFD_OKAY) {
            std::string file_path(outPath);
            std::cout << "[GRAN_AZUL] Selected project file: " << file_path << "\n";
            
            if (project_manager_->load_project(file_path)) {
                update_ui_from_project();
                std::cout << "[GRAN_AZUL] Project loaded successfully\n";
            } else {
                std::cout << "[GRAN_AZUL] Failed to load project file\n";
            }
            
            // Remember to free the path memory!
            NFD_FreePath(outPath);
            
        } else if (result == NFD_CANCEL) {
            std::cout << "[GRAN_AZUL] Project opening cancelled\n";
        } else {
            std::cout << "[GRAN_AZUL] Project opening error: " << NFD_GetError() << "\n";
        }
    }
    
    void save_project() {
        if (project_manager_->has_project()) {
            sync_project_from_ui();
            project_manager_->save_project();
        }
    }
    
    void save_project_as() {
        if (project_manager_->has_project()) {
            sync_project_from_ui();
            
            nfdchar_t *savePath;
            nfdfilteritem_t filterItem[1] = { { "Gran Azul Projects", "granazul" } };
            
            // Get current project name or use default
            std::string default_name = "Project.granazul";
            if (project_manager_->has_project()) {
                const auto& project = project_manager_->get_current_project();
                if (!project.name.empty()) {
                    default_name = project.name + ".granazul";
                }
            }
            
            nfdresult_t result = NFD_SaveDialog(&savePath, filterItem, 1, nullptr, default_name.c_str());
            
            if (result == NFD_OKAY) {
                std::string file_path(savePath);
                
                std::cout << "[GRAN_AZUL] Saving project to: " << file_path << "\n";
                
                if (project_manager_->save_project_as(file_path)) {
                    std::cout << "[GRAN_AZUL] Project saved successfully\n";
                } else {
                    std::cout << "[GRAN_AZUL] Failed to save project\n";
                }
                
                // Remember to free the path memory!
                NFD_FreePath(savePath);
                
            } else if (result == NFD_CANCEL) {
                std::cout << "[GRAN_AZUL] Save project cancelled\n";
            } else {
                std::cout << "[GRAN_AZUL] Save project error: " << NFD_GetError() << "\n";
            }
        }
    }
    
    void close_project() {
        project_manager_->close_project();
        // Reset UI to defaults
        std::cout << "[GRAN_AZUL] Project closed\n";
    }
    
    void update_ui_from_project() {
        if (project_manager_->has_project()) {
            const auto& project = project_manager_->get_current_project();
            
            // Update cppcheck widget configuration from project
            CppcheckConfig cppcheck_config;
            
            // Copy analysis settings from project to cppcheck config
            const auto& analysis = project.analysis;
            strcpy(cppcheck_config.source_path, analysis.source_path.c_str());
            strcpy(cppcheck_config.output_file, analysis.output_file.c_str());
            strcpy(cppcheck_config.build_dir, analysis.build_dir.c_str());
            
            cppcheck_config.enable_all = analysis.enable_all;
            cppcheck_config.enable_warning = analysis.enable_warning;
            cppcheck_config.enable_style = analysis.enable_style;
            cppcheck_config.enable_performance = analysis.enable_performance;
            cppcheck_config.enable_portability = analysis.enable_portability;
            cppcheck_config.enable_information = analysis.enable_information;
            cppcheck_config.enable_unused_function = analysis.enable_unused_function;
            cppcheck_config.enable_missing_include = analysis.enable_missing_include;
            
            cppcheck_config.check_level = analysis.check_level;
            cppcheck_config.inconclusive = analysis.inconclusive;
            cppcheck_config.verbose = analysis.verbose;
            cppcheck_config.cpp_standard = analysis.cpp_standard;
            cppcheck_config.platform = analysis.platform;
            cppcheck_config.job_count = analysis.job_count;
            cppcheck_config.quiet = analysis.quiet;
            
            cppcheck_config.suppress_unused_function = analysis.suppress_unused_function;
            cppcheck_config.suppress_missing_include_system = analysis.suppress_missing_include_system;
            cppcheck_config.suppress_missing_include = analysis.suppress_missing_include;
            cppcheck_config.suppress_duplicate_conditional = analysis.suppress_duplicate_conditional;
            cppcheck_config.use_posix_library = analysis.use_posix_library;
            cppcheck_config.use_misra_addon = analysis.use_misra_addon;
            
            cppcheck_widget_->set_config(cppcheck_config);
            
            std::cout << "[GRAN_AZUL] UI updated from project: " << project.name << std::endl;
        }
    }
    
    void sync_project_from_ui() {
        if (project_manager_->has_project()) {
            auto& project = project_manager_->get_current_project_mutable();
            const auto& cppcheck_config = cppcheck_widget_->get_config();
            
            // Update project analysis settings from UI
            auto& analysis = project.analysis;
            analysis.source_path = cppcheck_config.source_path;
            analysis.output_file = cppcheck_config.output_file;
            analysis.build_dir = cppcheck_config.build_dir;
            
            analysis.enable_all = cppcheck_config.enable_all;
            analysis.enable_warning = cppcheck_config.enable_warning;
            analysis.enable_style = cppcheck_config.enable_style;
            analysis.enable_performance = cppcheck_config.enable_performance;
            analysis.enable_portability = cppcheck_config.enable_portability;
            analysis.enable_information = cppcheck_config.enable_information;
            analysis.enable_unused_function = cppcheck_config.enable_unused_function;
            analysis.enable_missing_include = cppcheck_config.enable_missing_include;
            
            analysis.check_level = cppcheck_config.check_level;
            analysis.inconclusive = cppcheck_config.inconclusive;
            analysis.verbose = cppcheck_config.verbose;
            analysis.cpp_standard = cppcheck_config.cpp_standard;
            analysis.platform = cppcheck_config.platform;
            analysis.job_count = cppcheck_config.job_count;
            analysis.quiet = cppcheck_config.quiet;
            
            analysis.suppress_unused_function = cppcheck_config.suppress_unused_function;
            analysis.suppress_missing_include_system = cppcheck_config.suppress_missing_include_system;
            analysis.suppress_missing_include = cppcheck_config.suppress_missing_include;
            analysis.suppress_duplicate_conditional = cppcheck_config.suppress_duplicate_conditional;
            analysis.use_posix_library = cppcheck_config.use_posix_library;
            analysis.use_misra_addon = cppcheck_config.use_misra_addon;
            
            std::cout << "[GRAN_AZUL] Project updated from UI" << std::endl;
        }
    }
    
    std::string select_directory_dialog() {
        std::cout << "[GRAN_AZUL] Directory selection dialog requested\n";
        
        nfdchar_t *outPath;
        nfdfilteritem_t filterItem[1] = { { "Directory", "" } };
        
        nfdresult_t result = NFD_PickFolder(&outPath, nullptr);
        
        if (result == NFD_OKAY) {
            std::string directory_path(outPath);
            
            std::cout << "[GRAN_AZUL] Selected directory: " << directory_path << "\n";
            
            // Remember to free the path memory!
            NFD_FreePath(outPath);
            
            return directory_path;
        } else if (result == NFD_CANCEL) {
            std::cout << "[GRAN_AZUL] Directory selection cancelled\n";
        } else {
            std::cout << "[GRAN_AZUL] Directory selection error: " << NFD_GetError() << "\n";
        }
        
        return "";
    }
    
    void select_source_directory() {
        std::cout << "[GRAN_AZUL] Select source directory requested\n";
        
        nfdchar_t *outPath;
        nfdresult_t result = NFD_PickFolder(&outPath, nullptr);
        
        if (result == NFD_OKAY) {
            std::string directory_path(outPath);
            
            std::cout << "[GRAN_AZUL] Selected directory: " << directory_path << "\n";
            
            // Update the cppcheck configuration
            auto& config = const_cast<CppcheckConfig&>(cppcheck_widget_->get_config());
            strncpy(config.source_path, directory_path.c_str(), sizeof(config.source_path) - 1);
            config.source_path[sizeof(config.source_path) - 1] = '\0';
            
            // Update the widget with new configuration
            cppcheck_widget_->set_config(config);
            
            // If we have a project loaded, sync the changes
            if (project_manager_->has_project()) {
                sync_project_from_ui();
            }
            
            // Remember to free the path memory!
            NFD_FreePath(outPath);
            
        } else if (result == NFD_CANCEL) {
            std::cout << "[GRAN_AZUL] Directory selection cancelled\n";
        } else {
            std::cout << "[GRAN_AZUL] Directory selection error: " << NFD_GetError() << "\n";
        }
    }

    void render_menu_bar() {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Project", "Ctrl+N")) {
                    create_new_project();
                }
                if (ImGui::MenuItem("Open Project", "Ctrl+O")) {
                    open_project();
                }
                if (ImGui::MenuItem("Save Project", "Ctrl+S", nullptr, project_manager_->has_project())) {
                    save_project();
                }
                if (ImGui::MenuItem("Save Project As...", "Ctrl+Shift+S", nullptr, project_manager_->has_project())) {
                    save_project_as();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Close Project", nullptr, nullptr, project_manager_->has_project())) {
                    close_project();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Export Report", "Ctrl+E", nullptr, project_manager_->has_project())) {
                    std::cout << "[GRAN_AZUL] Export report requested\n";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    std::cout << "[GRAN_AZUL] Exit requested\n";
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Analysis")) {
                if (ImGui::MenuItem("Run Full Analysis", "F5")) {
                    auto& config = cppcheck_widget_->get_config();
                    run_cppcheck_analysis(config);
                }
                if (ImGui::MenuItem("Run Quick Scan", "Ctrl+F5")) {
                    std::cout << "[GRAN_AZUL] Quick scan requested\n";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Select Source Directory")) {
                    select_source_directory();
                }
                if (ImGui::MenuItem("Configure Cppcheck")) {
                    show_cppcheck_config = true;
                }
                if (ImGui::MenuItem("Test cppcheck --version")) {
                    run_cppcheck_version();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Configure Rules")) {
                    std::cout << "[GRAN_AZUL] Configure rules requested\n";
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("View")) {
                bool log_visible = log_panel_->is_visible();
                if (ImGui::MenuItem("Log Window", nullptr, &log_visible)) {
                    log_panel_->set_visible(log_visible);
                }
                
                bool analysis_visible = analysis_panel_->is_visible();
                if (ImGui::MenuItem("Analysis Results", nullptr, &analysis_visible)) {
                    analysis_panel_->set_visible(analysis_visible);
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About Gran Azul")) {
                    std::cout << "[GRAN_AZUL] About requested\n";
                }
                if (ImGui::MenuItem("Documentation", "F1")) {
                    std::cout << "[GRAN_AZUL] Documentation requested\n";
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }
    }

    void render_main_content() {
        // No main content panel - just use docked windows
        // Analysis results and log panels provide the main interface
    }
};

// Application class for Gran Azul
class GranAzulApp : public Application {
private:
    std::unique_ptr<GranAzulMainLayer> main_layer;
    
public:
    GranAzulApp() : Application("Gran Azul - Code Quality Analysis") {
        std::cout << "[GRAN_AZUL] Application initialized\n";
        
        // Create the main window
        create_window(1200, 800, "Gran Azul - Code Quality Analysis");
        
        // Create the layer but don't add it yet (ImGui context needed first)
        main_layer = std::make_unique<GranAzulMainLayer>();
    }
    
    void initialize() {
        // Initialize ImGui first
        initialize_imgui();
        
        // Now add the layer that uses ImGui
        add_layer(std::move(main_layer));
        
        std::cout << "[GRAN_AZUL] Application fully initialized\n";
    }
    
    ~GranAzulApp() {
        std::cout << "[GRAN_AZUL] Application shutting down\n";
    }
};

// Application entry point
int main() {
    try {
        std::cout << "[GRAN_AZUL] Starting Gran Azul Code Quality Analysis Platform...\n";
        
        // Create Gran Azul application
        GranAzulApp app;
        
        // Initialize ImGui and add layers
        app.initialize();
        
        std::cout << "[GRAN_AZUL] Controls:\n";
        std::cout << "[GRAN_AZUL]   ESC - Quit application\n";
        std::cout << "[GRAN_AZUL]   F1 - Show help\n";
        std::cout << "[GRAN_AZUL]   F11 - Toggle fullscreen\n";
        std::cout << "[GRAN_AZUL]   Ctrl+O - Open project\n";
        std::cout << "[GRAN_AZUL]   F5 - Run analysis\n";
        
        // Run the application loop
        app.run();
        
        std::cout << "[GRAN_AZUL] Application shut down successfully\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[GRAN_AZUL] Application error: " << e.what() << std::endl;
        return 1;
    }
}