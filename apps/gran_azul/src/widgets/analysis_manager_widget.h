#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

// Analysis library includes
#include "analysis_engine.h"
#include "analysis_types.h"
#include "tools/cppcheck_tool.h"
#include "tools/clang_tidy_tool.h"

// Project includes
#include "../project/project_config.h"

namespace gran_azul {
namespace widgets {

/**
 * @brief Unified analysis manager widget for multiple analysis tools
 * 
 * This widget replaces the old CppcheckConfigWidget and provides support for
 * multiple analysis tools (cppcheck, clang-tidy) through the analysis library.
 */
class AnalysisManagerWidget {
public:
    /**
     * @brief Callback for when analysis is requested
     * @param tool_names Names of tools to run
     * @param request Analysis request configuration
     */
    using AnalysisCallback = std::function<void(const std::vector<std::string>& tool_names, 
                                                const wip::analysis::AnalysisRequest& request)>;
    
    /**
     * @brief Callback for when tool version check is requested
     * @param tool_name Name of the tool to check
     */
    using VersionCallback = std::function<void(const std::string& tool_name)>;
    
    /**
     * @brief Callback for directory selection
     * @return Selected directory path, empty if cancelled
     */
    using DirectoryCallback = std::function<std::string()>;
    
    /**
     * @brief Callback for when configuration changes (to trigger project save)
     */
    using ConfigurationChangedCallback = std::function<void()>;

private:
    // Analysis engine
    std::unique_ptr<wip::analysis::AnalysisEngine> analysis_engine_;
    
    // UI state
    bool project_loaded_ = false;
    std::string project_base_path_;
    bool visible_ = false;
    
    // Analysis configuration
    char source_path_[512] = "./";
    char output_directory_[512] = "./analysis_results";
    std::vector<std::string> selected_tools_; // Will be populated based on tool_enabled_
    
    // Tool-specific configurations
    std::map<std::string, bool> tool_enabled_;
    
    // Cppcheck specific UI
    bool cppcheck_enable_all_ = true;
    bool cppcheck_enable_warning_ = true;
    bool cppcheck_enable_style_ = true;
    bool cppcheck_enable_performance_ = true;
    bool cppcheck_enable_portability_ = true;
    bool cppcheck_enable_information_ = false;
    int cppcheck_cpp_standard_ = 3; // C++17
    int cppcheck_job_count_ = 4;
    
    // Clang-tidy specific UI
    std::vector<std::string> clang_tidy_checks_ = {"bugprone-*", "performance-*", "readability-*"};
    char clang_tidy_checks_input_[1024] = "bugprone-*,performance-*,readability-*";
    bool clang_tidy_fix_errors_ = false;
    bool clang_tidy_header_filter_ = true;
    
    // Callbacks
    AnalysisCallback analysis_callback_;
    VersionCallback version_callback_;
    DirectoryCallback directory_callback_;
    ConfigurationChangedCallback config_changed_callback_;
    
    // UI helper methods
    void render_tool_selection();
    void render_common_configuration();
    void render_cppcheck_configuration();
    void render_clang_tidy_configuration();
    void render_analysis_controls();
    
    // Configuration methods
    wip::analysis::AnalysisRequest build_analysis_request() const;
    void configure_cppcheck_tool();
    void configure_clang_tidy_tool();
    
    // Utility methods
    std::vector<std::string> parse_clang_tidy_checks() const;

public:
    AnalysisManagerWidget();
    ~AnalysisManagerWidget() = default;
    
    /**
     * @brief Update the widget (call every frame)
     */
    void update(float delta_time);
    
    /**
     * @brief Render the widget
     */
    void render();
    
    /**
     * @brief Set whether a project is loaded
     */
    void set_project_loaded(bool loaded) { project_loaded_ = loaded; }
    
    /**
     * @brief Set the project base path
     */
    void set_project_base_path(const std::string& path);
    
    /**
     * @brief Load configuration from project
     */
    void load_from_project_config(const struct ProjectConfig& config);
    
    /**
     * @brief Save configuration to project
     */
    void save_to_project_config(struct ProjectConfig& config) const;
    
    /**
     * @brief Check if any analysis tools are available
     */
    bool has_available_tools() const;
    
    /**
     * @brief Get list of available analysis tools
     */
    std::vector<std::string> get_available_tools() const;
    
    /**
     * @brief Set analysis callback
     */
    void set_analysis_callback(AnalysisCallback callback) {
        analysis_callback_ = std::move(callback);
    }
    
    /**
     * @brief Set version check callback
     */
    void set_version_callback(VersionCallback callback) {
        version_callback_ = std::move(callback);
    }
    
    /**
     * @brief Set directory selection callback
     */
    void set_directory_callback(DirectoryCallback callback) {
        directory_callback_ = std::move(callback);
    }
    
    /**
     * @brief Set configuration changed callback (for auto-save)
     */
    void set_configuration_changed_callback(ConfigurationChangedCallback callback) {
        config_changed_callback_ = std::move(callback);
    }
    
    /**
     * @brief Set widget visibility
     */
    void set_visible(bool visible) {
        visible_ = visible;
    }
    
    /**
     * @brief Get widget visibility
     */
    bool is_visible() const {
        return visible_;
    }
};

} // namespace widgets
} // namespace gran_azul