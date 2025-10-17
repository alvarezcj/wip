#pragma once

#include "project_config.h"
#include <json_serializer.h>
#include <functional>
#include <memory>

namespace gran_azul {

using ProjectLoadCallback = std::function<void(const ProjectConfig& project)>;
using ProjectSaveCallback = std::function<void(const std::string& file_path)>;
using ProjectErrorCallback = std::function<void(const std::string& error_message)>;

/**
 * @brief Manages Gran Azul project files and configuration
 * 
 * The ProjectManager handles:
 * - Loading and saving project files (.granazul)
 * - Converting between ProjectConfig and JSON
 * - Managing current project state
 * - Project validation and error handling
 */
class ProjectManager {
private:
    std::unique_ptr<ProjectConfig> current_project_;
    std::string current_project_path_;
    
    // Callbacks for project events
    ProjectLoadCallback on_project_loaded_;
    ProjectSaveCallback on_project_saved_;
    ProjectErrorCallback on_error_;
    
    // JSON serializer
    wip::serialization::JsonSerializer<ProjectConfig> serializer_;
    
public:
    ProjectManager();
    ~ProjectManager() = default;
    
    // Project file operations
    bool create_new_project(const std::string& name, const std::string& root_path);
    bool load_project(const std::string& file_path);
    bool save_project();
    bool save_project_as(const std::string& file_path);
    bool close_project();
    
    // Project state management
    bool has_project() const { return current_project_ != nullptr; }
    const ProjectConfig& get_current_project() const;
    ProjectConfig& get_current_project_mutable();
    const std::string& get_current_project_path() const { return current_project_path_; }
    
    // Project validation
    bool is_project_valid() const;
    std::vector<std::string> get_project_validation_errors() const;
    
    // Utility methods
    static bool is_project_file(const std::string& file_path);
    static std::string get_default_project_filename(const std::string& project_name);
    static std::vector<std::string> find_project_files(const std::string& directory);
    
    // Configuration helpers
    void update_analysis_config(const ProjectConfig::AnalysisConfig& config);
    ProjectConfig::AnalysisConfig get_analysis_config() const;
    
    // Callback setters
    void set_project_loaded_callback(ProjectLoadCallback callback) { on_project_loaded_ = callback; }
    void set_project_saved_callback(ProjectSaveCallback callback) { on_project_saved_ = callback; }
    void set_error_callback(ProjectErrorCallback callback) { on_error_ = callback; }
    
private:
    void notify_project_loaded();
    void notify_project_saved();
    void notify_error(const std::string& message);
    
    bool write_project_file(const std::string& file_path, const ProjectConfig& project);
    std::optional<ProjectConfig> read_project_file(const std::string& file_path);
    
    void ensure_project_directories(const ProjectConfig& project);
    
    static constexpr const char* PROJECT_FILE_EXTENSION = ".granazul";
    static constexpr const char* PROJECT_FILE_VERSION = "1.0";
};

} // namespace gran_azul