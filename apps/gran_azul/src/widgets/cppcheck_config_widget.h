#pragma once

#include <widgets.h>
#include <process.h>
#include <string>
#include <vector>
#include <cstring>
#include <functional>

namespace gran_azul::widgets {

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
    
    // Output format is hardcoded to JSON for easy parsing
    bool verbose = false;
    
    // Standards and platform
    int cpp_standard = 4; // 0=c++03, 1=c++11, 2=c++14, 3=c++17, 4=c++20
    int platform = 1; // 0=unix32, 1=unix64, 2=win32A, 3=win64
    
    // Performance
    int job_count = 4;
    bool quiet = true;

    
    // Suppressions
    bool suppress_unused_function = true;
    bool suppress_missing_include_system = true;
    bool suppress_missing_include = true;  // Suppress general missing include warnings
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

// Callback types for the widget
using CppcheckAnalysisCallback = std::function<void(const CppcheckConfig&)>;
using CppcheckVersionCallback = std::function<void()>;
using CppcheckDirectoryCallback = std::function<void(const CppcheckConfig&)>;

class CppcheckConfigWidget : public wip::gui::Widget {
private:
    CppcheckConfig config_;
    CppcheckAnalysisCallback on_run_analysis_;
    CppcheckVersionCallback on_run_version_;
    CppcheckDirectoryCallback on_create_directory_;
    
public:
    CppcheckConfigWidget();
    
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
    
    // Utility methods
    std::string generate_command_preview() const;
    std::vector<std::string> generate_command_args() const;
    
private:
    void render_source_config();
    void render_analysis_options();
    void render_standards_platform();
    void render_performance_settings();
    void render_suppressions();
    void render_libraries_addons();
    void render_action_buttons();
    void render_command_preview();
};

} // namespace gran_azul::widgets