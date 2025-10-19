#include <application.h>
#include <layer.h>
#include <imgui.h>
#include <GL/gl.h>
#include <process.h>
#include <widgets.h>
#include "cppcheck_config_widget.h"
#include "analysis_manager_widget.h"
#include "log_window_panel.h"
#include "analysis_result_panel.h"
#include "analysis_result.h"
#include "progress_dialog.h"
#include "utils/async_process_executor.h"
#include "project_manager.h"
#include "report_generator.h"
#include "widgets/project_startup_modal.h"
#include "analysis_engine.h"
#include "analysis_types.h"
#include <nfd.h>
#include <memory>
#include <iostream>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <future>

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
    std::unique_ptr<CppcheckConfigWidget> cppcheck_widget_; // Keep for backward compatibility during transition
    std::unique_ptr<gran_azul::widgets::AnalysisManagerWidget> analysis_manager_;
    std::unique_ptr<LogWindowPanel> log_panel_;
    std::unique_ptr<AnalysisResultPanel> analysis_panel_;
    std::unique_ptr<gran_azul::widgets::ProgressDialog> progress_dialog_;
    std::unique_ptr<gran_azul::widgets::ProjectStartupModal> startup_modal_;
    
    // Async execution
    std::unique_ptr<gran_azul::utils::AsyncProcessExecutor> async_executor_;
    
    // Analysis engine for real-time analysis
    std::unique_ptr<wip::analysis::AnalysisEngine> current_analysis_engine_;
    
    // Analysis completion handling (for thread-safe UI updates)
    std::atomic<bool> analysis_completed_{false};
    std::mutex completion_data_mutex_;
    std::vector<wip::analysis::AnalysisResult> pending_results_;
    std::vector<std::string> pending_tool_names_;
    gran_azul::widgets::AnalysisResult pending_analysis_result_; // For single analysis results
    wip::utils::process::ProcessResult pending_result_; // Keep for legacy cppcheck
    CppcheckConfig pending_config_; // Keep for legacy cppcheck
    std::vector<std::string> pending_args_; // Keep for legacy cppcheck
    
    // Progress handling (for thread-safe UI updates)
    std::atomic<bool> progress_updated_{false};
    std::mutex progress_data_mutex_;
    struct ProgressData {
        std::string tool_name;
        float progress_ratio = 0.0f;
        std::string status_message;
        std::string current_file;
    };
    ProgressData pending_progress_;
    
    // Progress update throttling to prevent UI flooding
    std::chrono::steady_clock::time_point last_progress_update_;
    static constexpr std::chrono::milliseconds progress_update_interval_{100}; // Max 10 updates per second
    
    // Delayed analysis start (to ensure modal renders first)
    std::atomic<bool> start_analysis_next_frame_{false};
    std::vector<std::string> pending_analysis_tool_names_;
    wip::analysis::AnalysisRequest pending_analysis_request_;
    
    // Store the analysis future to keep it alive
    std::future<std::vector<wip::analysis::AnalysisResult>> current_analysis_future_;
    
    // Project management
    std::unique_ptr<gran_azul::ProjectManager> project_manager_;
    bool project_loaded_ = false;
    
    // Window state
    bool show_cppcheck_config = false;

public:
    GranAzulMainLayer() : Layer("GranAzul") {
        // Initialize progress update throttling
        last_progress_update_ = std::chrono::steady_clock::now();
        
        // Create widgets
        cppcheck_widget_ = std::make_unique<CppcheckConfigWidget>(); // Keep for compatibility
        analysis_manager_ = std::make_unique<gran_azul::widgets::AnalysisManagerWidget>();
        log_panel_ = std::make_unique<LogWindowPanel>();
        analysis_panel_ = std::make_unique<AnalysisResultPanel>("Analysis Results");
        progress_dialog_ = std::make_unique<gran_azul::widgets::ProgressDialog>("Analysis Progress");
        startup_modal_ = std::make_unique<gran_azul::widgets::ProjectStartupModal>();
        
        // Create async executor
        async_executor_ = std::make_unique<gran_azul::utils::AsyncProcessExecutor>();
        
        // Create project manager
        project_manager_ = std::make_unique<gran_azul::ProjectManager>();
        
        // Setup widget callbacks
        setup_widget_callbacks();
        setup_startup_modal_callbacks();
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
        // Debug: Track main thread activity during analysis
        static auto last_debug = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (now - last_debug > std::chrono::seconds(1)) {
            if (current_analysis_engine_ && current_analysis_engine_->is_analysis_running()) {
                std::cout << "[GRAN_AZUL] Main thread active during analysis (every 1s debug)\n";
            }
            last_debug = now;
        }
        
        // Check for delayed analysis start (to ensure modal renders first)
        if (start_analysis_next_frame_.load()) {
            start_pending_analysis();
        }
        
        // Check for progress updates and handle on main thread
        if (progress_updated_.load()) {
            handle_progress_update();
            progress_updated_.store(false);
        }
        
        // Check for completed analysis and handle on main thread
        if (analysis_completed_.load()) {
            std::cout << "[GRAN_AZUL] Main thread detected analysis completion\n";
            handle_analysis_completion();
            analysis_completed_.store(false);
        }
    }

    void on_render(Timestep timestep) override {
        // Always render the startup modal first if no project is loaded
        if (!project_loaded_) {
            if (startup_modal_->render()) {
                return; // Don't render main UI until project is loaded
            }
        }
        
        // Update widgets
        float delta_time = timestep.seconds();
        
        // Update project status in widgets
        bool has_project = project_manager_->has_project();
        cppcheck_widget_->set_project_loaded(has_project); // Keep for compatibility
        analysis_manager_->set_project_loaded(has_project);
        
        if (has_project) {
            std::filesystem::path project_dir = std::filesystem::path(project_manager_->get_current_project_path()).parent_path();
            cppcheck_widget_->set_project_base_path(project_dir.string()); // Keep for compatibility
            analysis_manager_->set_project_base_path(project_dir.string());
        }
        
        cppcheck_widget_->update(delta_time); // Keep for compatibility
        analysis_manager_->update(delta_time);
        log_panel_->update(delta_time);
        analysis_panel_->update(delta_time);
        progress_dialog_->update(delta_time);
        
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
        
        // Analysis manager window
        if (analysis_manager_->is_visible()) {
            analysis_manager_->render();
        }
        
        // Progress dialog (rendered on top)
        progress_dialog_->draw();
        
        // Cppcheck configuration window (legacy - keep for backward compatibility)
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
                        
                    case GLFW_KEY_F5:
                        if (project_manager_->has_project()) {
                            std::cout << "[GRAN_AZUL] F5 pressed - Run analysis\n";
                            auto& config = cppcheck_widget_->get_config();
                            run_cppcheck_analysis(config);
                        } else {
                            std::cout << "[GRAN_AZUL] F5 pressed - No project loaded\n";
                        }
                        return true;
                    
                    case GLFW_KEY_F11:
                        std::cout << "[GRAN_AZUL] F11 pressed - Toggle fullscreen\n";
                        return true;
                    
                    case GLFW_KEY_E:
                        if (key_event->is_ctrl() && project_manager_->has_project()) {
                            std::cout << "[GRAN_AZUL] Ctrl+E pressed - Generate report\n";
                            generate_comprehensive_report();
                            return true;
                        }
                        break;
                }
            }
        }
        
        return false; // Don't consume event by default
    }

private:
    void setup_widget_callbacks() {
        // Setup cppcheck widget callbacks (keep for compatibility)
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
        
        // Setup new analysis manager callbacks
        analysis_manager_->set_analysis_callback([this](const std::vector<std::string>& tool_names, 
                                                        const wip::analysis::AnalysisRequest& request) {
            run_analysis_with_library(tool_names, request);
        });
        
        analysis_manager_->set_version_callback([this](const std::string& tool_name) {
            check_tool_version(tool_name);
        });
        
        analysis_manager_->set_directory_callback([this]() -> std::string {
            return select_directory_dialog();
        });
        
        // Setup configuration change callback to auto-save project when user changes settings
        analysis_manager_->set_configuration_changed_callback([this]() {
            sync_project_from_ui();
            save_project();
        });
        
        // Setup analysis result panel callbacks
        analysis_panel_->set_file_open_callback([this](const std::string& file_path, int line, int column) {
            open_file_at_location(file_path, line, column);
        });
    }
    
    void setup_startup_modal_callbacks() {
        // Setup startup modal callbacks
        startup_modal_->set_new_project_callback([this](const std::string& project_name, 
                                                       const std::string& project_path,
                                                       const std::string& source_path) {
            create_new_project(project_name, project_path, source_path);
        });
        
        startup_modal_->set_load_project_callback([this](const std::string& project_file) {
            load_existing_project(project_file);
        });
        
        startup_modal_->set_exit_callback([this]() {
            // Signal the application to exit
            exit(0);
        });
    }
    
    void handle_progress_update() {
        std::lock_guard<std::mutex> lock(progress_data_mutex_);
        
        // Update progress dialog with thread-safe data
        if (!pending_progress_.status_message.empty()) {
            progress_dialog_->set_progress(pending_progress_.progress_ratio, pending_progress_.status_message);
            
            // Add output line for file processing
            if (!pending_progress_.current_file.empty()) {
                progress_dialog_->add_output_line("[" + pending_progress_.tool_name + "] Processing: " + pending_progress_.current_file);
            }
        }
    }
    
    void handle_analysis_completion() {
        std::cout << "[GRAN_AZUL] Handling analysis completion on main thread\n";
        std::lock_guard<std::mutex> lock(completion_data_mutex_);
        
        // Check if we have a new analysis result from the analysis library
        if (pending_analysis_result_.analysis_successful || !pending_analysis_result_.error_message.empty()) {
            std::cout << "[GRAN_AZUL] Processing analysis library result\n";
            
            // Update progress dialog
            progress_dialog_->set_completed(pending_analysis_result_.analysis_successful, 
                pending_analysis_result_.analysis_successful ? "Analysis completed successfully" : "Analysis completed with errors");
            
            // Display results in analysis panel
            analysis_panel_->set_analysis_result(pending_analysis_result_);
            
            // Print summary to console
            if (pending_analysis_result_.analysis_successful) {
                std::cout << "[GRAN_AZUL] Analysis Summary:\n";
                std::cout << "  - Total issues: " << pending_analysis_result_.issues.size() << "\n";
                std::cout << "  - Errors: " << pending_analysis_result_.count_by_severity(IssueSeverity::ERROR) << "\n";
                std::cout << "  - Warnings: " << pending_analysis_result_.count_by_severity(IssueSeverity::WARNING) << "\n";
                std::cout << "  - Style issues: " << pending_analysis_result_.count_by_severity(IssueSeverity::STYLE) << "\n";
                std::cout << "  - Performance issues: " << pending_analysis_result_.count_by_severity(IssueSeverity::PERFORMANCE) << "\n";
            } else {
                std::cout << "[GRAN_AZUL] Analysis failed: " << pending_analysis_result_.error_message << "\n";
            }
            
            // Clear the result
            pending_analysis_result_ = gran_azul::widgets::AnalysisResult{};
        } 
        // Fallback to legacy cppcheck handling
        else if (pending_result_.exit_code != -1) {
            std::cout << "[GRAN_AZUL] Processing legacy cppcheck result\n";
            
            // Create log entry
            std::string command_str = "cppcheck";
            for (const auto& arg : pending_args_) {
                command_str += " " + arg;
            }
            log_panel_->add_log_entry(command_str, pending_result_);
            
            std::cout << "[GRAN_AZUL] Cppcheck analysis completed with exit code: " << pending_result_.exit_code << "\n";
            std::cout << "[GRAN_AZUL] Output saved to: " << pending_config_.output_file << "\n";
            
            // Update progress dialog
            std::cout << "[GRAN_AZUL] Setting progress dialog as completed\n";
            progress_dialog_->set_completed(pending_result_.success(), 
                pending_result_.success() ? "Analysis completed successfully" : "Analysis completed with errors");
            
            // Parse and display analysis results
            parse_and_display_analysis_results(pending_config_);
            
            // Clear the result
            pending_result_ = wip::utils::process::ProcessResult{};
        }
        
        std::cout << "[GRAN_AZUL] Analysis completion handling finished\n";
    }
    
    void run_cppcheck_analysis(const CppcheckConfig& config) {
        std::cout << "[GRAN_AZUL] Starting async cppcheck analysis\n";
        
        if (!project_manager_->has_project()) {
            std::cout << "[GRAN_AZUL] No project loaded - cannot run analysis\n";
            return;
        }

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

        // If already running, show progress dialog
        if (async_executor_->is_running()) {
            progress_dialog_->show("Analysis already running...");
            return;
        }

        // Create a copy of config and modify output file to use project directory
        CppcheckConfig project_config = config;
        std::filesystem::path project_dir = std::filesystem::path(project_manager_->get_current_project_path()).parent_path();
        std::filesystem::path output_file = project_dir / "cppcheck_analysis.xml";
        strncpy(project_config.output_file, output_file.string().c_str(), sizeof(project_config.output_file) - 1);
        project_config.output_file[sizeof(project_config.output_file) - 1] = '\0';
        
        // Always create standardized build directory at .azul-cache/cppcheck-build
        std::filesystem::path azul_cache_dir = project_dir / ".azul-cache";
        std::filesystem::path build_dir_path = azul_cache_dir / "cppcheck-build";
        
        std::cout << "[GRAN_AZUL] Using standardized build directory: " << build_dir_path << "\n";
        
        // Create the cache and build directories
        std::error_code ec;
        std::filesystem::create_directories(build_dir_path, ec);
        if (ec) {
            std::cout << "[GRAN_AZUL] Failed to create build directory: " << ec.message() << "\n";
            ProcessResult error_result{1, "", "Failed to create build directory: " + ec.message(), std::chrono::milliseconds(0), false};
            log_panel_->add_log_entry("Create build directory", error_result);
            return;
        }
        
        // Update the config to use our standardized build directory
        strncpy(project_config.build_dir, build_dir_path.string().c_str(), sizeof(project_config.build_dir) - 1);
        project_config.build_dir[sizeof(project_config.build_dir) - 1] = '\0';
        
        // Generate command args with project-based output
        auto original_config = cppcheck_widget_->get_config();
        cppcheck_widget_->set_config(project_config);
        auto args = cppcheck_widget_->generate_command_args();
        cppcheck_widget_->set_config(original_config); // Restore original config for UI
        
        // Show progress dialog
        progress_dialog_->show("Initializing cppcheck analysis...");
        progress_dialog_->set_cancellable(true);
        
        // Setup async execution config
        gran_azul::utils::AsyncProcessConfig async_config;
        async_config.command = "cppcheck";
        async_config.arguments = args;
        async_config.working_directory = project_dir.string();
        async_config.parse_cppcheck_progress = true;
        
        // Setup callbacks
        async_config.on_progress = [this](float progress, const std::string& status) {
            progress_dialog_->set_progress(progress, status);
        };
        
        async_config.on_output = [this](const std::string& line) {
            progress_dialog_->add_output_line(line);
        };
        
        async_config.on_completion = [this, project_config, args](const wip::utils::process::ProcessResult& result) {
            std::cout << "[GRAN_AZUL] Background thread completion callback triggered\n";
            // Store completion data for main thread processing
            {
                std::lock_guard<std::mutex> lock(completion_data_mutex_);
                pending_result_ = result;
                pending_config_ = project_config;
                pending_args_ = args;
            }
            
            // Signal completion to main thread
            std::cout << "[GRAN_AZUL] Setting analysis_completed flag to true\n";
            analysis_completed_.store(true);
        };
        
        // Setup cancel callback
        progress_dialog_->set_cancel_callback([this]() {
            async_executor_->cancel();
        });
        
        // Start async execution
        auto future = async_executor_->execute_async(async_config);
    }
    
    void generate_comprehensive_report() {
        if (!project_manager_->has_project()) {
            std::cout << "[GRAN_AZUL] No project loaded - cannot generate report\n";
            return;
        }
        
        std::filesystem::path project_dir = std::filesystem::path(project_manager_->get_current_project_path()).parent_path();
        gran_azul::ReportGenerator generator(project_dir.string());
        
        // Analyze project structure
        const auto& project_config = project_manager_->get_current_project();
        std::vector<std::string> source_paths;
        if (strlen(cppcheck_widget_->get_config().source_path) > 0) {
            std::filesystem::path abs_source = project_dir / cppcheck_widget_->get_config().source_path;
            source_paths.push_back(abs_source.string());
        }
        
        gran_azul::ProjectSummary project_summary = gran_azul::ReportGenerator::analyze_project_structure(
            project_dir.string(), source_paths);
        project_summary.name = project_config.name;
        project_summary.project_file = project_manager_->get_current_project_path();
        
        // Generate report
        gran_azul::ComprehensiveReport report = generator.generate_report(project_summary);
        
        // Add cppcheck results if available
        std::filesystem::path cppcheck_output = project_dir / "cppcheck_analysis.xml";
        if (std::filesystem::exists(cppcheck_output)) {
            generator.add_cppcheck_results(report, cppcheck_output.string());
        }
        
        // Export reports with user-selected paths
        std::string json_path = select_json_report_save_path();
        if (json_path.empty()) {
            std::cout << "[GRAN_AZUL] JSON report export cancelled\n";
            return;
        }
        
        std::string html_path = select_html_report_save_path();
        if (html_path.empty()) {
            std::cout << "[GRAN_AZUL] HTML report export cancelled\n";
            return;
        }
        
        bool json_success = generator.export_json_report(report, json_path);
        bool html_success = generator.export_html_report(report, html_path);
        
        if (json_success && html_success) {
            std::cout << "[GRAN_AZUL] Comprehensive reports generated successfully\n";
            std::cout << "[GRAN_AZUL] JSON: " << json_path << "\n";
            std::cout << "[GRAN_AZUL] HTML: " << html_path << "\n";
        } else {
            std::cout << "[GRAN_AZUL] Some reports failed to generate\n";
        }
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
    
    // Helper function to convert analysis library result to widget result
    gran_azul::widgets::AnalysisResult convert_analysis_result(const wip::analysis::AnalysisResult& lib_result) {
        gran_azul::widgets::AnalysisResult widget_result;
        
        // Copy basic fields
        widget_result.analysis_successful = lib_result.success;
        widget_result.error_message = lib_result.error_message;
        widget_result.source_path = ""; // Not directly available in lib result
        widget_result.total_files_analyzed = static_cast<int>(lib_result.files_analyzed);
        
        // Set timestamp (convert from time_point to string)
        auto time_t = std::chrono::system_clock::to_time_t(lib_result.timestamp);
        widget_result.timestamp = std::asctime(std::localtime(&time_t));
        
        // Convert issues
        for (const auto& lib_issue : lib_result.issues) {
            gran_azul::widgets::AnalysisIssue widget_issue;
            
            widget_issue.file = lib_issue.file_path;
            widget_issue.line = lib_issue.line_number;
            widget_issue.column = lib_issue.column_number;
            widget_issue.id = lib_issue.rule_id;
            widget_issue.message = lib_issue.message;
            widget_issue.cwe = 0; // Not available in lib result
            widget_issue.false_positive = false;
            
            // Convert severity (different enum values)
            switch (lib_issue.severity) {
                case wip::analysis::IssueSeverity::Error:
                case wip::analysis::IssueSeverity::Critical:
                    widget_issue.severity = gran_azul::widgets::IssueSeverity::ERROR;
                    break;
                case wip::analysis::IssueSeverity::Warning:
                    widget_issue.severity = gran_azul::widgets::IssueSeverity::WARNING;
                    break;
                case wip::analysis::IssueSeverity::Info:
                default:
                    // Map based on category for better classification
                    switch (lib_issue.category) {
                        case wip::analysis::IssueCategory::Performance:
                            widget_issue.severity = gran_azul::widgets::IssueSeverity::PERFORMANCE;
                            break;
                        case wip::analysis::IssueCategory::Style:
                            widget_issue.severity = gran_azul::widgets::IssueSeverity::STYLE;
                            break;
                        case wip::analysis::IssueCategory::Portability:
                            widget_issue.severity = gran_azul::widgets::IssueSeverity::PORTABILITY;
                            break;
                        default:
                            widget_issue.severity = gran_azul::widgets::IssueSeverity::INFORMATION;
                            break;
                    }
                    break;
            }
            
            widget_result.issues.push_back(widget_issue);
        }
        
        return widget_result;
    }
    
    // Function to merge multiple analysis results into one
    gran_azul::widgets::AnalysisResult merge_analysis_results(const std::vector<wip::analysis::AnalysisResult>& results) {
        gran_azul::widgets::AnalysisResult merged_result;
        
        if (results.empty()) {
            merged_result.analysis_successful = false;
            merged_result.error_message = "No analysis results returned";
            return merged_result;
        }
        
        // Start with success - will be false if any tool failed
        merged_result.analysis_successful = true;
        merged_result.total_files_analyzed = 0;
        std::vector<std::string> error_messages;
        
        // Get latest timestamp
        auto latest_timestamp = std::chrono::system_clock::time_point::min();
        
        for (const auto& result : results) {
            // Merge basic info
            if (!result.success) {
                merged_result.analysis_successful = false;
                if (!result.error_message.empty()) {
                    error_messages.push_back(result.tool_name + ": " + result.error_message);
                }
            }
            
            merged_result.total_files_analyzed += static_cast<int>(result.files_analyzed);
            
            if (result.timestamp > latest_timestamp) {
                latest_timestamp = result.timestamp;
            }
            
            // Convert and merge issues
            for (const auto& lib_issue : result.issues) {
                gran_azul::widgets::AnalysisIssue widget_issue;
                
                widget_issue.file = lib_issue.file_path;
                widget_issue.line = lib_issue.line_number;
                widget_issue.column = lib_issue.column_number;
                widget_issue.id = lib_issue.rule_id;
                widget_issue.message = lib_issue.message;
                widget_issue.cwe = 0; // Not available in lib result
                widget_issue.false_positive = false;
                
                // Convert severity (different enum values)
                switch (lib_issue.severity) {
                    case wip::analysis::IssueSeverity::Error:
                    case wip::analysis::IssueSeverity::Critical:
                        widget_issue.severity = gran_azul::widgets::IssueSeverity::ERROR;
                        break;
                    case wip::analysis::IssueSeverity::Warning:
                        widget_issue.severity = gran_azul::widgets::IssueSeverity::WARNING;
                        break;
                    case wip::analysis::IssueSeverity::Info:
                    default:
                        // Map based on category for better classification
                        switch (lib_issue.category) {
                            case wip::analysis::IssueCategory::Performance:
                                widget_issue.severity = gran_azul::widgets::IssueSeverity::PERFORMANCE;
                                break;
                            case wip::analysis::IssueCategory::Style:
                                widget_issue.severity = gran_azul::widgets::IssueSeverity::STYLE;
                                break;
                            case wip::analysis::IssueCategory::Portability:
                                widget_issue.severity = gran_azul::widgets::IssueSeverity::PORTABILITY;
                                break;
                            default:
                                widget_issue.severity = gran_azul::widgets::IssueSeverity::INFORMATION;
                                break;
                        }
                        break;
                }
                
                merged_result.issues.push_back(widget_issue);
            }
        }
        
        // Set timestamp (convert from time_point to string)
        auto time_t = std::chrono::system_clock::to_time_t(latest_timestamp);
        merged_result.timestamp = std::asctime(std::localtime(&time_t));
        
        // Combine error messages
        if (!error_messages.empty()) {
            merged_result.error_message = "";
            for (size_t i = 0; i < error_messages.size(); ++i) {
                if (i > 0) merged_result.error_message += "; ";
                merged_result.error_message += error_messages[i];
            }
        }
        
        return merged_result;
    }
    
    void run_analysis_with_library(const std::vector<std::string>& tool_names, const wip::analysis::AnalysisRequest& request) {
        std::cout << "[GRAN_AZUL] Starting analysis with library for " << tool_names.size() << " tools\n";
        std::cout << "[GRAN_AZUL] Tools: ";
        for (const auto& tool : tool_names) {
            std::cout << tool << " ";
        }
        std::cout << "\n";
        std::cout << "[GRAN_AZUL] Source path: " << request.source_path << "\n";
        std::cout << "[GRAN_AZUL] Output file: " << request.output_file << "\n";
        
        if (!project_manager_->has_project()) {
            std::cout << "[GRAN_AZUL] No project loaded - cannot run analysis\n";
            return;
        }
        
        // Check if already running
        if (current_analysis_engine_ && current_analysis_engine_->is_analysis_running()) {
            progress_dialog_->show("Analysis already running...");
            return;
        }
        
        try {
            // Get project directory
            std::filesystem::path project_dir = std::filesystem::path(project_manager_->get_current_project_path()).parent_path();
            std::cout << "[GRAN_AZUL] Project directory: " << project_dir << "\n";
            
            // Create analysis engine
            current_analysis_engine_ = wip::analysis::AnalysisEngineFactory::create_engine_with_tools(tool_names);
            if (!current_analysis_engine_) {
                std::cout << "[GRAN_AZUL] Failed to create analysis engine\n";
                gran_azul::widgets::AnalysisResult error_result;
                error_result.analysis_successful = false;
                error_result.error_message = "Failed to create analysis engine";
                analysis_panel_->set_analysis_result(error_result);
                return;
            }
            
            std::cout << "[GRAN_AZUL] Analysis engine created with " << current_analysis_engine_->get_registered_tools().size() << " tools\n";
            
            // Show progress dialog
            std::string tools_str = "";
            for (size_t i = 0; i < tool_names.size(); ++i) {
                if (i > 0) tools_str += ", ";
                tools_str += tool_names[i];
            }
            progress_dialog_->show("Running " + tools_str + " analysis...");
            progress_dialog_->set_cancellable(true);
            
            // Setup cancel callback for analysis engine
            progress_dialog_->set_cancel_callback([this]() {
                if (current_analysis_engine_) {
                    current_analysis_engine_->cancel_analysis();
                }
            });
            
            // Force render one frame to ensure modal appears before starting analysis
            std::cout << "[GRAN_AZUL] Modal shown, will start analysis after UI update...\n";
            
            // Store analysis parameters to start in next update cycle
            pending_analysis_tool_names_ = tool_names;
            pending_analysis_request_ = request;
            start_analysis_next_frame_ = true;
            
            return; // Exit here, analysis will start in next update cycle
        } catch (const std::exception& e) {
            std::cout << "[GRAN_AZUL] Exception starting analysis: " << e.what() << "\n";
            gran_azul::widgets::AnalysisResult error_result;
            error_result.analysis_successful = false;
            error_result.error_message = e.what();
            analysis_panel_->set_analysis_result(error_result);
        }
    }
    
    // Helper method to actually start the analysis (called from update loop)
    void start_pending_analysis() {
        if (!start_analysis_next_frame_) return;
        
        start_analysis_next_frame_ = false;
        std::cout << "[GRAN_AZUL] Starting delayed analysis with " << pending_analysis_tool_names_.size() << " tools\n";
        
        // Run analysis asynchronously with progress callbacks
        auto progress_callback = [this](const std::string& tool_name, const wip::analysis::AnalysisProgress& progress) {
            // Rate limit progress updates to prevent UI flooding
            auto now = std::chrono::steady_clock::now();
            bool should_update = false;
            
            {
                std::lock_guard<std::mutex> lock(progress_data_mutex_);
                
                // Only update if enough time has passed since last update
                if (now - last_progress_update_ >= progress_update_interval_) {
                    pending_progress_.tool_name = tool_name;
                    pending_progress_.progress_ratio = static_cast<float>(progress.get_progress_ratio());
                    pending_progress_.status_message = progress.status_message;
                    if (!progress.current_file.empty()) {
                        pending_progress_.status_message += " (" + progress.current_file + ")";
                        pending_progress_.current_file = progress.current_file;
                    } else {
                        pending_progress_.current_file.clear();
                    }
                    
                    last_progress_update_ = now;
                    should_update = true;
                }
            }
            
            // Only signal update if we actually updated the data
            if (should_update) {
                progress_updated_.store(true);
            }
        };
        
        auto completion_callback = [this](const std::vector<wip::analysis::AnalysisResult>& results) {
            std::cout << "[GRAN_AZUL] Analysis completed with " << results.size() << " results\n";
            
            // Store merged results for main thread processing
            {
                std::lock_guard<std::mutex> lock(completion_data_mutex_);
                pending_analysis_result_ = merge_analysis_results(results);
                
                std::cout << "[GRAN_AZUL] Merged result with " << pending_analysis_result_.issues.size() << " total issues\n";
            }
            
            // Signal completion
            analysis_completed_.store(true);
        };
        
        // Start async analysis with callbacks
        try {
            current_analysis_future_ = current_analysis_engine_->analyze_async(pending_analysis_tool_names_, pending_analysis_request_, progress_callback, completion_callback);
            
            // Future is now stored and will keep the analysis alive
            std::cout << "[GRAN_AZUL] Analysis future created and stored, analysis running in background\n";
            
        } catch (const std::exception& e) {
            std::cout << "[GRAN_AZUL] Failed to start delayed async analysis: " << e.what() << "\n";
            
            // Store error result
            {
                std::lock_guard<std::mutex> lock(completion_data_mutex_);
                pending_analysis_result_ = gran_azul::widgets::AnalysisResult{};
                pending_analysis_result_.analysis_successful = false;
                pending_analysis_result_.error_message = e.what();
            }
            
            analysis_completed_.store(true);
        }
    }
    
    void check_tool_version(const std::string& tool_name) {
        std::cout << "[GRAN_AZUL] Checking version for: " << tool_name << "\n";
        
        try {
            // Create analysis engine
            auto engine = wip::analysis::AnalysisEngineFactory::create_engine_with_tools({tool_name});
            if (!engine) {
                ProcessResult error_result{127, "", tool_name + " engine not available", std::chrono::milliseconds(0), false};
                log_panel_->add_log_entry(tool_name + " version check", error_result);
                return;
            }
            
            // Get the tool and check its version
            auto* tool = engine->get_tool(tool_name);
            if (!tool) {
                ProcessResult error_result{127, "", tool_name + " tool not found in engine", std::chrono::milliseconds(0), false};
                log_panel_->add_log_entry(tool_name + " version check", error_result);
                return;
            }
            
            // Get version info
            std::string version = tool->get_version();
            
            // Create mock ProcessResult for compatibility with log panel
            ProcessResult result{0, version, "", std::chrono::milliseconds(100), true};
            log_panel_->add_log_entry(tool_name + " --version", result);
            
            std::cout << "[GRAN_AZUL] " << tool_name << " version: " << version << "\n";
            
        } catch (const std::exception& e) {
            std::cout << "[GRAN_AZUL] Error checking " << tool_name << " version: " << e.what() << "\n";
            ProcessResult error_result{1, "", e.what(), std::chrono::milliseconds(0), false};
            log_panel_->add_log_entry(tool_name + " version check", error_result);
        }
    }
    
    void render_cppcheck_config_window() {
        if (first_frame) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.1f, viewport->WorkPos.y + viewport->WorkSize.y * 0.1f));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.8f, viewport->WorkSize.y * 0.8f));
        }
        
        if (ImGui::Begin("Cppcheck Configuration", &show_cppcheck_config, ImGuiWindowFlags_AlwaysAutoResize)) {
            cppcheck_widget_->draw();
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
        project_loaded_ = false;
        // Reset UI to defaults
        std::cout << "[GRAN_AZUL] Project closed\n";
    }
    
    // New methods for startup modal
    void create_new_project(const std::string& project_name, 
                           const std::string& project_path,
                           const std::string& source_path) {
        std::cout << "[GRAN_AZUL] Creating new project: " << project_name << "\n";
        std::cout << "[GRAN_AZUL] Project path: " << project_path << "\n";
        std::cout << "[GRAN_AZUL] Source path: " << source_path << "\n";
        
        try {
            // Create the project using the project manager
            if (project_manager_->create_new_project(project_name, project_path)) {
                // Update the project configuration with the selected source path
                auto& project = project_manager_->get_current_project_mutable();
                project.name = project_name;
                project.root_path = project_path;
                project.analysis.source_path = source_path;
                
                // Set output file relative to project directory
                std::filesystem::path project_dir(project_path);
                std::filesystem::path output_file = project_dir / "analysis_results.xml";
                project.analysis.output_file = output_file.string();
                
                // Create the project file
                std::filesystem::path project_file = project_dir / (project_name + ".granazul");
                if (project_manager_->save_project_as(project_file.string())) {
                    project_loaded_ = true;
                    update_ui_from_project();
                    std::cout << "[GRAN_AZUL] New project created successfully: " << project_file << "\n";
                } else {
                    startup_modal_->show_error("Failed to save project file");
                    std::cout << "[GRAN_AZUL] Failed to save project file\n";
                }
            } else {
                startup_modal_->show_error("Failed to create project");
                std::cout << "[GRAN_AZUL] Failed to create project\n";
            }
        } catch (const std::exception& e) {
            startup_modal_->show_error("Error creating project: " + std::string(e.what()));
            std::cout << "[GRAN_AZUL] Exception creating project: " << e.what() << "\n";
        }
    }
    
    void load_existing_project(const std::string& project_file) {
        std::cout << "[GRAN_AZUL] Loading existing project: " << project_file << "\n";
        
        try {
            if (project_manager_->load_project(project_file)) {
                project_loaded_ = true;
                update_ui_from_project();
                std::cout << "[GRAN_AZUL] Project loaded successfully\n";
            } else {
                startup_modal_->show_error("Failed to load project file. The file may be corrupted or invalid.");
                std::cout << "[GRAN_AZUL] Failed to load project file\n";
            }
        } catch (const std::exception& e) {
            startup_modal_->show_error("Error loading project: " + std::string(e.what()));
            std::cout << "[GRAN_AZUL] Exception loading project: " << e.what() << "\n";
        }
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
            
            // Update analysis manager from project
            analysis_manager_->load_from_project_config(project);
            analysis_manager_->set_project_base_path(std::filesystem::path(project_manager_->get_current_project_path()).parent_path().string());
            analysis_manager_->set_visible(true); // Show analysis manager when project is loaded
            
            std::cout << "[GRAN_AZUL] UI updated from project: " << project.name << std::endl;
        }
    }
    
    void sync_project_from_ui() {
        if (project_manager_->has_project()) {
            auto& project = project_manager_->get_current_project_mutable();
            
            // Update from the new AnalysisManagerWidget (takes priority)
            analysis_manager_->save_to_project_config(project);
            
            // Legacy support: also sync from old CppcheckWidget if needed
            const auto& cppcheck_config = cppcheck_widget_->get_config();
            
            // Update project analysis settings from UI
            auto& analysis = project.analysis;
            // Note: source_path and basic settings now come from AnalysisManagerWidget above
            
            analysis.build_dir = cppcheck_config.build_dir;
            analysis.enable_unused_function = cppcheck_config.enable_unused_function;
            analysis.enable_missing_include = cppcheck_config.enable_missing_include;
            
            analysis.check_level = cppcheck_config.check_level;
            analysis.inconclusive = cppcheck_config.inconclusive;
            analysis.verbose = cppcheck_config.verbose;
            analysis.platform = cppcheck_config.platform;
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
    
    std::string select_json_report_save_path() {
        std::cout << "[GRAN_AZUL] JSON report save dialog requested\n";
        
        nfdchar_t *savePath;
        nfdfilteritem_t filterItem[1] = { { "JSON Files", "json" } };
        
        nfdresult_t result = NFD_SaveDialog(&savePath, filterItem, 1, nullptr, "quality_report.json");
        
        if (result == NFD_OKAY) {
            std::string file_path(savePath);
            std::cout << "[GRAN_AZUL] Selected JSON report path: " << file_path << "\n";
            NFD_FreePath(savePath);
            return file_path;
        } else if (result == NFD_CANCEL) {
            std::cout << "[GRAN_AZUL] JSON report save cancelled\n";
        } else {
            std::cout << "[GRAN_AZUL] JSON report save error: " << NFD_GetError() << "\n";
        }
        
        return "";
    }
    
    std::string select_html_report_save_path() {
        std::cout << "[GRAN_AZUL] HTML report save dialog requested\n";
        
        nfdchar_t *savePath;
        nfdfilteritem_t filterItem[1] = { { "HTML Files", "html" } };
        
        nfdresult_t result = NFD_SaveDialog(&savePath, filterItem, 1, nullptr, "quality_report.html");
        
        if (result == NFD_OKAY) {
            std::string file_path(savePath);
            std::cout << "[GRAN_AZUL] Selected HTML report path: " << file_path << "\n";
            NFD_FreePath(savePath);
            return file_path;
        } else if (result == NFD_CANCEL) {
            std::cout << "[GRAN_AZUL] HTML report save cancelled\n";
        } else {
            std::cout << "[GRAN_AZUL] HTML report save error: " << NFD_GetError() << "\n";
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
                if (ImGui::MenuItem("Generate Report", "Ctrl+E", nullptr, project_manager_->has_project())) {
                    generate_comprehensive_report();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    std::cout << "[GRAN_AZUL] Exit requested\n";
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Analysis")) {
                bool has_project = project_manager_->has_project();
                if (ImGui::MenuItem("Analysis Manager", "F6", nullptr, has_project)) {
                    analysis_manager_->set_visible(true);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Run Full Analysis", "F5", nullptr, has_project)) {
                    auto& config = cppcheck_widget_->get_config();
                    run_cppcheck_analysis(config);
                }
                if (ImGui::MenuItem("Run Quick Scan", "Ctrl+F5", nullptr, has_project)) {
                    std::cout << "[GRAN_AZUL] Quick scan requested\n";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Select Source Directory", nullptr, nullptr, has_project)) {
                    select_source_directory();
                }
                if (ImGui::MenuItem("Configure Cppcheck (Legacy)", nullptr, nullptr, has_project)) {
                    show_cppcheck_config = true;
                }
                if (ImGui::MenuItem("Test cppcheck --version")) {
                    run_cppcheck_version();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Configure Rules", nullptr, nullptr, has_project)) {
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
                
                bool analysis_manager_visible = analysis_manager_->is_visible();
                if (ImGui::MenuItem("Analysis Manager", nullptr, &analysis_manager_visible)) {
                    analysis_manager_->set_visible(analysis_manager_visible);
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
    int a = 0;
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
        std::cout << "[GRAN_AZUL]   Ctrl+E - Generate report\n";
        
        // Run the application loop
        app.run();
        
        std::cout << "[GRAN_AZUL] Application shut down successfully\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[GRAN_AZUL] Application error: " << e.what() << std::endl;
        return 1;
    }
}