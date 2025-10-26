#pragma once

#include <widgets.h>
#include <process.h>
#include <string>
#include <vector>
#include <cstring>
#include <functional>
#include <memory>

// Forward declare to avoid circular dependency
namespace gran_azul::widgets { class PathSelectorWidget; }

namespace gran_azul::widgets {

// Cppcheck configuration structure
struct CppcheckConfig {
    char source_path[512] = "";
    char output_file[512] = "cppcheck_analysis.xml";
    char build_dir[512] = "";
    
    // Custom include paths and definitions
    std::vector<std::string> include_paths;
    std::vector<std::string> preprocessor_definitions;
    
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
    
    // Output format is hardcoded to JSON for easy parsing
    bool verbose = false;
    
    // Standards and platform
    int cpp_standard = 4; // 0=c++03, 1=c++11, 2=c++14, 3=c++17, 4=c++20
    int platform = 1; // 0=unix32, 1=unix64, 2=win32A, 3=win64
    
    // Performance
    int job_count = 4;
    bool quiet = false;

    
    // Suppressions
    bool suppress_unused_function = true;
    bool suppress_missing_include_system = true;
    bool suppress_missing_include = true;  // Suppress general missing include warnings
    bool suppress_duplicate_conditional = false;
    
    // Libraries
    bool use_posix_library = true;
    bool use_misra_addon = false;
    
    CppcheckConfig() {
        // Set empty defaults for new projects
        strcpy(source_path, "");
        strcpy(output_file, "cppcheck_analysis.xml");
        strcpy(build_dir, "");
    }
};

// Callback types for the widget
using CppcheckAnalysisCallback = std::function<void(const CppcheckConfig&)>;
using CppcheckVersionCallback = std::function<void()>;
using CppcheckDirectoryCallback = std::function<void(const CppcheckConfig&)>;
using CppcheckDirectorySelectionCallback = std::function<std::string()>;

class CppcheckConfigWidget : public wip::gui::Widget {
private:
    CppcheckConfig config_;
    CppcheckAnalysisCallback on_run_analysis_;
    CppcheckVersionCallback on_run_version_;
    CppcheckDirectoryCallback on_create_directory_;
    CppcheckDirectorySelectionCallback on_select_directory_;
    bool has_project_ = false;
    std::string project_base_path_;
    
    // Path selectors
    std::unique_ptr<PathSelectorWidget> source_path_selector_;
    std::unique_ptr<PathSelectorWidget> build_path_selector_;
    
    // UI state for advanced configuration
    char new_include_path_[512] = "";
    char new_preprocessor_def_[256] = "";
    
public:
    CppcheckConfigWidget();
    ~CppcheckConfigWidget(); // Custom destructor needed for unique_ptr with incomplete type
    
    // Widget interface
    void update(float delta_time) override;
    void draw() override;
    
    // Configuration access
    const CppcheckConfig& get_config() const { return config_; }
    void set_config(const CppcheckConfig& config) { config_ = config; }
    
    // Callback setters
    void set_analysis_callback(CppcheckAnalysisCallback callback) { on_run_analysis_ = callback; }
    void set_version_callback(CppcheckVersionCallback callback) { on_run_version_ = callback; }
    void set_directory_callback(CppcheckDirectoryCallback callback) { on_create_directory_ = callback; }
    void set_directory_selection_callback(CppcheckDirectorySelectionCallback callback) { on_select_directory_ = callback; }
    
    // Project status
    void set_project_loaded(bool has_project) { has_project_ = has_project; }
    void set_project_base_path(const std::string& base_path) { project_base_path_ = base_path; }
    
    // Utility methods
    std::string generate_command_preview() const;
    std::vector<std::string> generate_command_args() const;
    
private:
    void render_source_config();
    void render_include_paths();
    void render_preprocessor_definitions();
    void render_analysis_options();
    void render_standards_platform();
    void render_performance_settings();
    void render_suppressions();
    void render_libraries_addons();
    void render_action_buttons();
    void render_command_preview();
    
    // Helper methods for relative path handling
    std::string convert_to_relative_path(const std::string& absolute_path);
    std::string get_project_base_path();
};

} // namespace gran_azul::widgets