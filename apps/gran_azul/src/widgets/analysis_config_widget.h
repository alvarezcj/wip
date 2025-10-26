#pragma once

#include <widgets.h>
#include <string>
#include <functional>
#include "../project/project_config.h"

namespace gran_azul::widgets {

/**
 * @brief Unified analysis configuration widget for both cppcheck and clang-tidy
 */
class AnalysisConfigWidget : public wip::gui::Widget {
public:
    // Callback types
    using ConfigChangedCallback = std::function<void(const ProjectConfig::AnalysisConfig&)>;
    using AnalysisCallback = std::function<void(const std::vector<std::string>& tool_names)>;
    using VersionCheckCallback = std::function<void(const std::string& tool_name)>;
    using DirectorySelectionCallback = std::function<std::string()>;

private:
    ProjectConfig::AnalysisConfig config_;
    bool project_loaded_ = false;
    std::string project_base_path_;
    
    // UI state
    int selected_tab_ = 0;  // 0=Tools, 1=Cppcheck, 2=Clang-Tidy, 3=Common
    bool show_advanced_cppcheck_ = false;
    bool show_advanced_clang_tidy_ = false;
    
    // Callbacks
    ConfigChangedCallback on_config_changed_;
    AnalysisCallback on_analysis_requested_;
    VersionCheckCallback on_version_check_;
    DirectorySelectionCallback on_directory_selection_;

public:
    explicit AnalysisConfigWidget();
    
    // Widget interface
    void update(float delta_time) override;
    void draw() override;
    
    void render(gran_azul::ProjectConfig::AnalysisConfig& config);
    
private:
    
    // Configuration management
    void set_config(const ProjectConfig::AnalysisConfig& config);
    const ProjectConfig::AnalysisConfig& get_config() const { return config_; }
    
    // Project integration
    void set_project_loaded(bool loaded) { project_loaded_ = loaded; }
    void set_project_base_path(const std::string& path) { project_base_path_ = path; }
    
    // Callback setters
    void set_config_changed_callback(ConfigChangedCallback callback) { on_config_changed_ = callback; }
    void set_analysis_callback(AnalysisCallback callback) { on_analysis_requested_ = callback; }
    void set_version_check_callback(VersionCheckCallback callback) { on_version_check_ = callback; }
    void set_directory_selection_callback(DirectorySelectionCallback callback) { on_directory_selection_ = callback; }
    
    // State queries
    bool is_project_loaded() const { return project_loaded_; }
    std::vector<std::string> get_enabled_tools() const;
    
private:
    // UI rendering methods
    void render_tools_tab(gran_azul::ProjectConfig::AnalysisConfig& config);
    void render_tool_selection_tab(gran_azul::ProjectConfig::AnalysisConfig& config);
    void render_cppcheck_tab(gran_azul::ProjectConfig::AnalysisConfig& config);
    void render_clang_tidy_tab(gran_azul::ProjectConfig::AnalysisConfig& config);
    void render_common_tab(gran_azul::ProjectConfig::AnalysisConfig& config);
    
    void render_cppcheck_basic_options();
    void render_cppcheck_advanced_options();
    void render_clang_tidy_checks();
    void render_clang_tidy_options();
    void render_common_options();
    
    // Utility methods
    void trigger_config_changed();
    std::string get_cpp_standard_string(int standard) const;
    std::string get_platform_string(int platform) const;
    bool is_tool_available(const std::string& tool_name) const;
    
    // Input helpers
    bool input_text_project_relative(const char* label, std::string& path, const char* hint = nullptr);
};

} // namespace gran_azul::widgets