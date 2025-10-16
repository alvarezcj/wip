#include <application.h>
#include <layer.h>
#include <imgui.h>
#include <GL/gl.h>
#include <process.h>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstring>

using namespace wip::gui::application;
using namespace wip::gui::window::events;
using namespace wip::utils::process;

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

// Log entry structure for the log window
struct LogEntry {
    std::string timestamp;
    std::string command;
    std::string stdout_output;
    std::string stderr_output;
    bool success;
    int exit_code;
    std::chrono::milliseconds duration;
    
    LogEntry(const std::string& cmd, const ProcessResult& result) 
        : command(cmd), stdout_output(result.stdout_output), 
          stderr_output(result.stderr_output), success(result.success()), 
          exit_code(result.exit_code), duration(result.duration) {
        // Create timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::ostringstream ss;
        ss << std::put_time(&tm, "%H:%M:%S");
        timestamp = ss.str();
    }
};

// Cppcheck configuration structure
struct CppcheckConfig {
    char source_path[512] = "./";
    char output_file[512] = "analysis_results.xml";
    char build_dir[512] = "./cppcheck_build";
    
    // Analysis options
    bool enable_all = true;
    bool enable_warning = true;
    bool enable_style = true;
    bool enable_performance = true;
    bool enable_portability = true;
    bool enable_information = false;
    bool enable_unused_function = false;
    bool enable_missing_include = false;
    
    // Analysis level
    int check_level = 0; // 0 = normal, 1 = exhaustive
    bool inconclusive = false;
    
    // Output format
    int output_format = 0; // 0 = XML, 1 = JSON-like, 2 = CSV, 3 = Text, 4 = Custom
    char custom_template[512] = "{file}:{line}:{column}: {severity}: [{id}] {message}";
    bool verbose = false;
    
    // Standards and platform
    int cpp_standard = 4; // 0=c++03, 1=c++11, 2=c++14, 3=c++17, 4=c++20
    int platform = 1; // 0=unix32, 1=unix64, 2=win32A, 3=win64
    
    // Performance
    int job_count = 4;
    bool quiet = true;
    bool show_timing = false;
    
    // Suppressions
    bool suppress_unused_function = true;
    bool suppress_missing_include_system = true;
    bool suppress_duplicate_conditional = false;
    
    // Libraries
    bool use_posix_library = true;
    bool use_misra_addon = false;
    
    CppcheckConfig() {
        // Set default source path to gran_azul
        strcpy(source_path, "apps/gran_azul/src/");
        strcpy(output_file, "gran_azul_analysis.xml");
        strcpy(build_dir, "./cppcheck_build");
    }
};

// Main application layer for Gran Azul
class GranAzulMainLayer : public Layer {
private:
    bool first_frame = true;
    GranAzulTheme current_theme;
    FontSystem font_system;
    ProcessExecutor process_executor;
    std::vector<LogEntry> log_entries;
    bool show_log_window = true;
    bool all_collapsed = false;
    bool force_collapse_state = false; // Flag to force state change
    bool show_cppcheck_config = false;
    CppcheckConfig cppcheck_config;

public:
    GranAzulMainLayer() : Layer("GranAzul") {
        // Note: Don't load fonts or apply themes here - ImGui context not ready yet
    }

    void on_attach() override {
        std::cout << "[GRAN_AZUL] Application layer attached\n";
        
        // Now it's safe to initialize ImGui-dependent components
        font_system.load_fonts();
        apply_gran_azul_theme();
        
        std::cout << "[GRAN_AZUL] ImGui components initialized\n";
    }

    void on_detach() override {
        std::cout << "[GRAN_AZUL] Application layer detached\n";
    }

    void on_update(Timestep timestep) override {
        // Application logic updates will go here
        // For now, just a basic update loop
    }

    void on_render(Timestep timestep) override {
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
        if (show_log_window) {
            render_log_window();
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
    void run_cppcheck_analysis() {
        std::cout << "[GRAN_AZUL] Running cppcheck analysis with configuration\n";
        
        if (!ProcessExecutor::command_exists("cppcheck")) {
            LogEntry entry("cppcheck analysis", ProcessResult{127, "", "cppcheck command not found", std::chrono::milliseconds(0), false});
            log_entries.push_back(entry);
            return;
        }
        
        // Build command arguments based on configuration
        std::vector<std::string> args;
        
        // Enable checks
        if (cppcheck_config.enable_all) {
            args.push_back("--enable=all");
        } else {
            std::string enables = "--enable=";
            bool first = true;
            if (cppcheck_config.enable_warning) {
                if (!first) enables += ",";
                enables += "warning";
                first = false;
            }
            if (cppcheck_config.enable_style) {
                if (!first) enables += ",";
                enables += "style";
                first = false;
            }
            if (cppcheck_config.enable_performance) {
                if (!first) enables += ",";
                enables += "performance";
                first = false;
            }
            if (cppcheck_config.enable_portability) {
                if (!first) enables += ",";
                enables += "portability";
                first = false;
            }
            if (cppcheck_config.enable_information) {
                if (!first) enables += ",";
                enables += "information";
                first = false;
            }
            if (cppcheck_config.enable_unused_function) {
                if (!first) enables += ",";
                enables += "unusedFunction";
                first = false;
            }
            if (cppcheck_config.enable_missing_include) {
                if (!first) enables += ",";
                enables += "missingInclude";
                first = false;
            }
            if (!first) {
                args.push_back(enables);
            }
        }
        
        // Analysis level and options
        if (cppcheck_config.check_level == 1) {
            args.push_back("--check-level=exhaustive");
        }
        if (cppcheck_config.inconclusive) {
            args.push_back("--inconclusive");
        }
        if (cppcheck_config.verbose) {
            args.push_back("--verbose");
        }
        
        // Output format and template
        switch (cppcheck_config.output_format) {
            case 0: // XML
                args.push_back("--xml");
                args.push_back("--output-file=" + std::string(cppcheck_config.output_file));
                break;
            case 1: // JSON-like
                args.push_back("--template={\"file\":\"{file}\",\"line\":{line},\"column\":{column},\"severity\":\"{severity}\",\"id\":\"{id}\",\"message\":\"{message}\",\"cwe\":{cwe}}");
                args.push_back("--output-file=" + std::string(cppcheck_config.output_file));
                break;
            case 2: // CSV
                args.push_back("--template={file},{line},{column},{severity},{id},\"{message}\",{cwe}");
                args.push_back("--output-file=" + std::string(cppcheck_config.output_file));
                break;
            case 3: // Text
                args.push_back("--template={file}:{line}:{column}: {severity}: [{id}] {message} (CWE-{cwe})");
                args.push_back("--output-file=" + std::string(cppcheck_config.output_file));
                break;
            case 4: // Custom Template
                args.push_back("--template=" + std::string(cppcheck_config.custom_template));
                args.push_back("--output-file=" + std::string(cppcheck_config.output_file));
                break;
        }
        
        // Standards and platform
        const char* standards[] = {"c++03", "c++11", "c++14", "c++17", "c++20"};
        args.push_back("--std=" + std::string(standards[cppcheck_config.cpp_standard]));
        
        const char* platforms[] = {"unix32", "unix64", "win32A", "win64"};
        args.push_back("--platform=" + std::string(platforms[cppcheck_config.platform]));
        
        // Performance options
        if (cppcheck_config.job_count > 1) {
            args.push_back("-j");
            args.push_back(std::to_string(cppcheck_config.job_count));
        }
        if (cppcheck_config.quiet) {
            args.push_back("--quiet");
        }
        if (cppcheck_config.show_timing) {
            args.push_back("--showtime=summary");
        }
        
        // Libraries and addons
        if (cppcheck_config.use_posix_library) {
            args.push_back("--library=posix");
        }
        if (cppcheck_config.use_misra_addon) {
            args.push_back("--addon=misra");
        }
        
        // Build directory
        if (strlen(cppcheck_config.build_dir) > 0) {
            args.push_back("--cppcheck-build-dir=" + std::string(cppcheck_config.build_dir));
        }
        
        // Suppressions
        if (cppcheck_config.suppress_unused_function) {
            args.push_back("--suppress=unusedFunction");
        }
        if (cppcheck_config.suppress_missing_include_system) {
            args.push_back("--suppress=missingIncludeSystem");
        }
        if (cppcheck_config.suppress_duplicate_conditional) {
            args.push_back("--suppress=duplicateConditionalAssign");
        }
        
        // Source path (last argument)
        args.push_back(cppcheck_config.source_path);
        
        // Execute the command
        auto result = process_executor.execute("cppcheck", args);
        
        // Create log entry
        std::string command_str = "cppcheck";
        for (const auto& arg : args) {
            command_str += " " + arg;
        }
        LogEntry entry(command_str, result);
        log_entries.push_back(entry);
        
        std::cout << "[GRAN_AZUL] Cppcheck analysis completed with exit code: " << result.exit_code << "\n";
        std::cout << "[GRAN_AZUL] Output saved to: " << cppcheck_config.output_file << "\n";
    }
    
    void render_cppcheck_config_window() {
        if (first_frame) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.1f, viewport->WorkPos.y + viewport->WorkSize.y * 0.1f));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.8f, viewport->WorkSize.y * 0.8f));
        }
        
        if (ImGui::Begin("Cppcheck Configuration", &show_cppcheck_config, ImGuiWindowFlags_AlwaysAutoResize)) {
            // Header
            ImGui::Text("Configure Cppcheck Static Analysis");
            ImGui::Separator();
            
            // Source Path Configuration
            if (ImGui::CollapsingHeader("Source Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Source Path:");
                ImGui::InputText("##source_path", cppcheck_config.source_path, sizeof(cppcheck_config.source_path));
                if (ImGui::Button("Select Directory")) {
                    // TODO: Implement directory picker
                    std::cout << "[GRAN_AZUL] Directory picker not implemented yet\n";
                }
                
                ImGui::Spacing();
                ImGui::Text("Output File:");
                ImGui::InputText("##output_file", cppcheck_config.output_file, sizeof(cppcheck_config.output_file));
                
                ImGui::Spacing();
                ImGui::Text("Build Directory (optional):");
                ImGui::InputText("##build_dir", cppcheck_config.build_dir, sizeof(cppcheck_config.build_dir));
                ImGui::SameLine();
                if (ImGui::Button("Create")) {
                    // Create build directory
                    std::string cmd = "mkdir -p " + std::string(cppcheck_config.build_dir);
                    auto result = process_executor.execute(cmd);
                    LogEntry entry("mkdir -p " + std::string(cppcheck_config.build_dir), result);
                    log_entries.push_back(entry);
                }
            }
            
            // Analysis Options
            if (ImGui::CollapsingHeader("Analysis Options", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Enable All Checks##enable_all", &cppcheck_config.enable_all);
                
                if (!cppcheck_config.enable_all) {
                    ImGui::Indent();
                    ImGui::Checkbox("Warnings##enable_warning", &cppcheck_config.enable_warning);
                    ImGui::Checkbox("Style##enable_style", &cppcheck_config.enable_style);
                    ImGui::Checkbox("Performance##enable_performance", &cppcheck_config.enable_performance);
                    ImGui::Checkbox("Portability##enable_portability", &cppcheck_config.enable_portability);
                    ImGui::Checkbox("Information##enable_information", &cppcheck_config.enable_information);
                    ImGui::Checkbox("Unused Functions##enable_unused_function", &cppcheck_config.enable_unused_function);
                    ImGui::Checkbox("Missing Includes##enable_missing_include", &cppcheck_config.enable_missing_include);
                    ImGui::Unindent();
                }
                
                ImGui::Spacing();
                ImGui::Text("Analysis Level:");
                const char* levels[] = {"Normal", "Exhaustive"};
                ImGui::Combo("##check_level", &cppcheck_config.check_level, levels, IM_ARRAYSIZE(levels));
                
                ImGui::Checkbox("Include Inconclusive Results##inconclusive", &cppcheck_config.inconclusive);
                ImGui::Checkbox("Verbose Output##verbose", &cppcheck_config.verbose);
            }
            
            // Output Format
            if (ImGui::CollapsingHeader("Output Format", ImGuiTreeNodeFlags_DefaultOpen)) {
                const char* formats[] = {"XML", "JSON-like", "CSV", "Text", "Custom Template"};
                ImGui::Combo("##output_format", &cppcheck_config.output_format, formats, IM_ARRAYSIZE(formats));
                ImGui::SameLine();
                ImGui::Text("Output Format");
                
                // Show custom template input when Custom Template is selected
                if (cppcheck_config.output_format == 4) {
                    ImGui::Spacing();
                    ImGui::Text("Custom Template:");
                    ImGui::InputText("##custom_template", cppcheck_config.custom_template, sizeof(cppcheck_config.custom_template));
                    ImGui::TextWrapped("Available placeholders: {file}, {line}, {column}, {severity}, {id}, {message}, {cwe}");
                }
                
                ImGui::Checkbox("Show Timing Information##show_timing", &cppcheck_config.show_timing);
            }
            
            // Standards and Platform
            if (ImGui::CollapsingHeader("Standards & Platform")) {
                const char* standards[] = {"C++03", "C++11", "C++14", "C++17", "C++20"};
                ImGui::Combo("##cpp_standard", &cppcheck_config.cpp_standard, standards, IM_ARRAYSIZE(standards));
                ImGui::SameLine();
                ImGui::Text("C++ Standard");
                
                const char* platforms[] = {"Unix 32-bit", "Unix 64-bit", "Windows 32-bit", "Windows 64-bit"};
                ImGui::Combo("##platform", &cppcheck_config.platform, platforms, IM_ARRAYSIZE(platforms));
                ImGui::SameLine();
                ImGui::Text("Target Platform");
            }
            
            // Performance Settings
            if (ImGui::CollapsingHeader("Performance Settings")) {
                ImGui::SliderInt("##job_count", &cppcheck_config.job_count, 1, 16);
                ImGui::SameLine();
                ImGui::Text("Job Count (Threads)");
                ImGui::Checkbox("Quiet Mode##quiet", &cppcheck_config.quiet);
            }
            
            // Suppressions
            if (ImGui::CollapsingHeader("Suppressions")) {
                ImGui::Checkbox("Suppress Unused Functions##suppress_unused", &cppcheck_config.suppress_unused_function);
                ImGui::Checkbox("Suppress Missing System Includes##suppress_missing", &cppcheck_config.suppress_missing_include_system);
                ImGui::Checkbox("Suppress Duplicate Conditionals##suppress_duplicate", &cppcheck_config.suppress_duplicate_conditional);
            }
            
            // Libraries and Addons
            if (ImGui::CollapsingHeader("Libraries & Addons")) {
                ImGui::Checkbox("Use POSIX Library##use_posix", &cppcheck_config.use_posix_library);
                ImGui::Checkbox("Use MISRA Addon##use_misra", &cppcheck_config.use_misra_addon);
            }
            
            // Action Buttons
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("Run Analysis", ImVec2(120, 0))) {
                run_cppcheck_analysis();
                // Keep window open to allow viewing results and running again
            }
            ImGui::SameLine();
            if (ImGui::Button("Test cppcheck --version", ImVec2(160, 0))) {
                run_cppcheck_version();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset to Defaults", ImVec2(140, 0))) {
                cppcheck_config = CppcheckConfig(); // Reset to defaults
            }
            ImGui::SameLine();
            if (ImGui::Button("Close", ImVec2(80, 0))) {
                show_cppcheck_config = false;
            }
            
            // Show command preview
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Command Preview:");
            ImGui::BeginChild("CommandPreview", ImVec2(0, 100), true);
            
            // Generate preview command
            std::string preview = "cppcheck";
            
            // Enable checks
            if (cppcheck_config.enable_all) {
                preview += " --enable=all";
            } else {
                std::string enables = "";
                bool first = true;
                if (cppcheck_config.enable_warning) {
                    if (!first) enables += ",";
                    enables += "warning";
                    first = false;
                }
                if (cppcheck_config.enable_style) {
                    if (!first) enables += ",";
                    enables += "style";
                    first = false;
                }
                if (cppcheck_config.enable_performance) {
                    if (!first) enables += ",";
                    enables += "performance";
                    first = false;
                }
                if (cppcheck_config.enable_portability) {
                    if (!first) enables += ",";
                    enables += "portability";
                    first = false;
                }
                if (cppcheck_config.enable_information) {
                    if (!first) enables += ",";
                    enables += "information";
                    first = false;
                }
                if (cppcheck_config.enable_unused_function) {
                    if (!first) enables += ",";
                    enables += "unusedFunction";
                    first = false;
                }
                if (cppcheck_config.enable_missing_include) {
                    if (!first) enables += ",";
                    enables += "missingInclude";
                    first = false;
                }
                if (!first) {
                    preview += " --enable=" + enables;
                }
            }
            
            // Analysis level and options
            if (cppcheck_config.check_level == 1) {
                preview += " --check-level=exhaustive";
            }
            if (cppcheck_config.inconclusive) {
                preview += " --inconclusive";
            }
            if (cppcheck_config.verbose) {
                preview += " --verbose";
            }
            
            // Standards and platform
            const char* standards[] = {"c++03", "c++11", "c++14", "c++17", "c++20"};
            preview += " --std=" + std::string(standards[cppcheck_config.cpp_standard]);
            
            const char* platforms[] = {"unix32", "unix64", "win32A", "win64"};
            preview += " --platform=" + std::string(platforms[cppcheck_config.platform]);
            
            // Performance options
            if (cppcheck_config.job_count > 1) {
                preview += " -j " + std::to_string(cppcheck_config.job_count);
            }
            if (cppcheck_config.quiet) {
                preview += " --quiet";
            }
            if (cppcheck_config.show_timing) {
                preview += " --showtime=summary";
            }
            
            // Libraries and addons
            if (cppcheck_config.use_posix_library) {
                preview += " --library=posix";
            }
            if (cppcheck_config.use_misra_addon) {
                preview += " --addon=misra";
            }
            
            // Build directory
            if (strlen(cppcheck_config.build_dir) > 0) {
                preview += " --cppcheck-build-dir=" + std::string(cppcheck_config.build_dir);
            }
            
            // Suppressions
            if (cppcheck_config.suppress_unused_function) {
                preview += " --suppress=unusedFunction";
            }
            if (cppcheck_config.suppress_missing_include_system) {
                preview += " --suppress=missingIncludeSystem";
            }
            if (cppcheck_config.suppress_duplicate_conditional) {
                preview += " --suppress=duplicateConditionalAssign";
            }
            
            // Output format
            switch (cppcheck_config.output_format) {
                case 0: // XML
                    preview += " --xml --output-file=" + std::string(cppcheck_config.output_file);
                    break;
                case 1: // JSON-like
                    preview += " --template={\"file\":\"{file}\",\"line\":{line},\"severity\":\"{severity}\",\"message\":\"{message}\"}";
                    break;
                case 2: // CSV
                    preview += " --template={file},{line},{severity},\"{message}\"";
                    break;
                case 3: // Text
                    preview += " --template={file}:{line}: {severity}: {message}";
                    break;
                case 4: // Custom Template
                    preview += " --template=" + std::string(cppcheck_config.custom_template);
                    break;
            }
            
            // Source path (last argument)
            preview += " " + std::string(cppcheck_config.source_path);
            
            ImGui::TextWrapped("%s", preview.c_str());
            ImGui::EndChild();
        }
        ImGui::End();
    }

    void run_cppcheck_version() {
        std::cout << "[GRAN_AZUL] Running cppcheck --version\n";
        
        if (!ProcessExecutor::command_exists("cppcheck")) {
            LogEntry entry("cppcheck --version", ProcessResult{127, "", "cppcheck command not found", std::chrono::milliseconds(0), false});
            log_entries.push_back(entry);
            return;
        }
        
        std::vector<std::string> args = {"--version"};
        auto result = process_executor.execute("cppcheck", args);
        
        LogEntry entry("cppcheck --version", result);
        log_entries.push_back(entry);
        
        std::cout << "[GRAN_AZUL] cppcheck --version completed with exit code: " << result.exit_code << "\n";
    }
    
    void render_selectable_text(const std::string& text, size_t entry_index, const std::string& type) {
        // Split text into lines and make each line selectable
        std::string text_copy = text;
        std::string line;
        size_t line_num = 0;
        size_t start = 0;
        size_t end = 0;
        
        while ((end = text_copy.find('\n', start)) != std::string::npos) {
            line = text_copy.substr(start, end - start);
            if (ImGui::Selectable((line + "##line_" + type + "_" + std::to_string(entry_index) + "_" + std::to_string(line_num)).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    ImGui::SetClipboardText(line.c_str());
                    std::cout << "[GRAN_AZUL] Line copied to clipboard\n";
                }
            }
            start = end + 1;
            ++line_num;
        }
        
        // Handle the last line (or the only line if no newlines)
        if (start < text_copy.length()) {
            line = text_copy.substr(start);
            if (ImGui::Selectable((line + "##line_" + type + "_" + std::to_string(entry_index) + "_" + std::to_string(line_num)).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    ImGui::SetClipboardText(line.c_str());
                    std::cout << "[GRAN_AZUL] Line copied to clipboard\n";
                }
            }
        }
    }

    void render_log_window() {
        if (first_frame) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y * 0.6f));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y * 0.4f));
        }
        
        if (ImGui::Begin("Command Log", &show_log_window)) {
            // Header with clear button and collapse all
            if (ImGui::Button("Clear Log")) {
                log_entries.clear();
                std::cout << "[GRAN_AZUL] Log cleared\n";
            }
            ImGui::SameLine();
            if (ImGui::Button(all_collapsed ? "Expand All" : "Collapse All")) {
                all_collapsed = !all_collapsed;
                force_collapse_state = true; // Force state change on next render
                std::cout << "[GRAN_AZUL] " << (all_collapsed ? "Collapsed" : "Expanded") << " all log entries\n";
            }
            ImGui::SameLine();
            ImGui::Text("Commands executed: %zu", log_entries.size());
            
            ImGui::Separator();
            
            // Log content - scroll to bottom
            ImGuiWindowFlags child_flags = ImGuiWindowFlags_HorizontalScrollbar;
            if (ImGui::BeginChild("LogContent", ImVec2(0, 0), false, child_flags)) {
                // Iterate in reverse order to show latest commands at the top
                for (int idx = static_cast<int>(log_entries.size()) - 1; idx >= 0; --idx) {
                    size_t i = static_cast<size_t>(idx);
                    const auto& entry = log_entries[i];
                    
                    // Header with timestamp and command
                    ImGui::PushID(static_cast<int>(i));
                    
                    // Color-code based on success
                    ImVec4 color = entry.success ? ImVec4(0.0f, 0.8f, 0.0f, 1.0f) : ImVec4(0.8f, 0.0f, 0.0f, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    
                    // Force the collapse state if the button was pressed
                    if (force_collapse_state) {
                        ImGui::SetNextItemOpen(!all_collapsed);
                    }
                    
                    bool is_open = ImGui::CollapsingHeader(("Command " + std::to_string(log_entries.size() - i) + " - " + entry.timestamp).c_str());
                    
                    if (is_open) {
                        ImGui::PopStyleColor(); // Reset color for content
                        
                        // Display command in a scrollable, copyable area
                        ImGui::Text("Command:");
                        ImGui::SameLine();
                        if (ImGui::Button(("Copy Command##cmd_" + std::to_string(i)).c_str())) {
                            ImGui::SetClipboardText(entry.command.c_str());
                            std::cout << "[GRAN_AZUL] Command copied to clipboard\n";
                        }
                        
                        ImGui::BeginChild(("CommandText_" + std::to_string(i)).c_str(), ImVec2(0, 60), true);
                        render_selectable_text(entry.command, i, "command");
                        ImGui::EndChild();
                        
                        ImGui::Text("Exit Code: %d", entry.exit_code);
                        ImGui::Text("Duration: %ld ms", entry.duration.count());
                        ImGui::Text("Success: %s", entry.success ? "Yes" : "No");
                        
                        ImGui::Separator();
                        
                        // Display stdout
                        if (!entry.stdout_output.empty()) {
                            ImGui::Text("Standard Output:");
                            
                            // Add copy button for stdout
                            ImGui::SameLine();
                            if (ImGui::Button(("Copy Stdout##stdout_" + std::to_string(i)).c_str())) {
                                ImGui::SetClipboardText(entry.stdout_output.c_str());
                                std::cout << "[GRAN_AZUL] Stdout copied to clipboard\n";
                            }
                            
                            // Display stdout in a selectable child window
                            ImGui::BeginChild(("StdoutText_" + std::to_string(i)).c_str(), ImVec2(0, 100), true);
                            render_selectable_text(entry.stdout_output, i, "stdout");
                            ImGui::EndChild();
                        }
                        
                        // Display stderr
                        if (!entry.stderr_output.empty()) {
                            if (!entry.stdout_output.empty()) {
                                ImGui::Spacing();
                            }
                            
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 0.0f, 1.0f)); // Orange for stderr
                            ImGui::Text("Standard Error:");
                            ImGui::PopStyleColor();
                            
                            // Add copy button for stderr
                            ImGui::SameLine();
                            if (ImGui::Button(("Copy Stderr##stderr_" + std::to_string(i)).c_str())) {
                                ImGui::SetClipboardText(entry.stderr_output.c_str());
                                std::cout << "[GRAN_AZUL] Stderr copied to clipboard\n";
                            }
                            
                            // Display stderr in a selectable child window
                            ImGui::BeginChild(("StderrText_" + std::to_string(i)).c_str(), ImVec2(0, 100), true);
                            render_selectable_text(entry.stderr_output, i, "stderr");
                            ImGui::EndChild();
                        }
                        
                        // If both are empty
                        if (entry.stdout_output.empty() && entry.stderr_output.empty()) {
                            ImGui::Text("No output");
                        }
                    } else {
                        ImGui::PopStyleColor(); // Reset color if header is collapsed
                    }
                    
                    ImGui::PopID();
                    ImGui::Spacing();
                }
                
                // Reset the force flag after processing all headers
                force_collapse_state = false;
                
                // Auto-scroll to bottom when new entries are added
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                    ImGui::SetScrollHereY(1.0f);
                }
            }
            ImGui::EndChild();
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

    void render_menu_bar() {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open Project", "Ctrl+O")) {
                    std::cout << "[GRAN_AZUL] Open project requested\n";
                }
                if (ImGui::MenuItem("Open Recent")) {
                    std::cout << "[GRAN_AZUL] Open recent requested\n";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Export Report", "Ctrl+E")) {
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
                    std::cout << "[GRAN_AZUL] Full analysis requested\n";
                }
                if (ImGui::MenuItem("Run Quick Scan", "Ctrl+F5")) {
                    std::cout << "[GRAN_AZUL] Quick scan requested\n";
                }
                ImGui::Separator();
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
                ImGui::MenuItem("Log Window", nullptr, &show_log_window);
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
        // Main content window
        if (first_frame) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + 19));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - 19));
        }
        
        ImGui::Begin("Gran Azul - C/C++ Code Quality Analysis");
        
        // Welcome header with medium font
        if (font_system.medium_font) {
            ImGui::PushFont(font_system.medium_font, 0.0f);
            ImGui::Text("Gran Azul - Code Quality Analysis Platform");
            ImGui::PopFont();
        } else {
            ImGui::Text("Gran Azul - Code Quality Analysis Platform");
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Welcome message
        ImGui::Text("Welcome to Gran Azul, your comprehensive C/C++ code quality analysis tool.");
        ImGui::Text("This platform aggregates and facilitates multiple analysis tools to provide");
        ImGui::Text("unified reporting, quality gates, and actionable insights for your codebase.");
        
        ImGui::Spacing();
        
        // Current status
        if (ImGui::CollapsingHeader("Quick Start", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            
            ImGui::Text("Get started with your code quality analysis:");
            ImGui::BulletText("Open a C/C++ project using File -> Open Project");
            ImGui::BulletText("Configure analysis rules and quality gates");
            ImGui::BulletText("Run analysis using Analysis -> Run Full Analysis");
            ImGui::BulletText("Review results and export reports");
            
            ImGui::Spacing();
            
            // Action buttons
            if (ImGui::Button("Open Project", ImVec2(120, 0))) {
                std::cout << "[GRAN_AZUL] Open project button clicked\n";
            }
            ImGui::SameLine();
            if (ImGui::Button("Cppcheck Config", ImVec2(120, 0))) {
                show_cppcheck_config = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Run Analysis", ImVec2(120, 0))) {
                run_cppcheck_analysis();
            }
            
            ImGui::Separator();
        }
        
        if (ImGui::CollapsingHeader("Supported Analysis Tools", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            
            ImGui::Text("Gran Azul integrates the following industry-standard tools:");
            ImGui::BulletText("cppcheck - Static analysis and bug detection");
            ImGui::BulletText("clang-tidy - Clang-based linting and modernization");
            ImGui::BulletText("clang-static-analyzer - Advanced static analysis");
            ImGui::BulletText("PC-lint Plus - Commercial-grade analysis (coming soon)");
            ImGui::BulletText("Valgrind integration - Memory error detection");
            
            ImGui::Separator();
        }
        
        if (ImGui::CollapsingHeader("Analysis Features")) {
            ImGui::Spacing();
            
            ImGui::Text("Comprehensive analysis capabilities:");
            ImGui::BulletText("Static code analysis with configurable rules");
            ImGui::BulletText("Security vulnerability detection");
            ImGui::BulletText("Code complexity analysis");
            ImGui::BulletText("MISRA C/C++ compliance checking");
            ImGui::BulletText("Memory leak and buffer overflow detection");
            ImGui::BulletText("Quality gates and severity classification");
            
            ImGui::Separator();
        }
        
        // Status area
        ImGui::Spacing();
        ImGui::Text("Status: Ready for analysis");
        ImGui::Text("Version: 1.0.0 (Initial Release)");
        
        ImGui::End();
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