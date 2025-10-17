#include "project_manager.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace gran_azul {

ProjectManager::ProjectManager() = default;

bool ProjectManager::create_new_project(const std::string& name, const std::string& root_path) {
    try {
        // Create new project config with defaults
        auto project = std::make_unique<ProjectConfig>();
        project->name = name;
        project->root_path = std::filesystem::absolute(root_path).string();
        
        // Set up default paths relative to root
        project->analysis.source_path = "./apps";
        project->analysis.output_file = name + "_analysis.json";
        project->analysis.build_dir = "./cppcheck_build";
        project->reports_directory = "./reports";
        
        // Validate the project
        if (!project->is_valid()) {
            notify_error("Invalid project configuration");
            return false;
        }
        
        // Ensure directories exist
        ensure_project_directories(*project);
        
        // Set as current project
        current_project_ = std::move(project);
        current_project_path_.clear(); // New project, not yet saved
        
        notify_project_loaded();
        return true;
    } catch (const std::exception& e) {
        notify_error("Failed to create project: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::load_project(const std::string& file_path) {
    try {
        if (!std::filesystem::exists(file_path)) {
            notify_error("Project file does not exist: " + file_path);
            return false;
        }
        
        auto project_config = read_project_file(file_path);
        if (!project_config) {
            notify_error("Failed to parse project file: " + file_path);
            return false;
        }
        
        // Update root path to be relative to project file location
        std::filesystem::path project_dir = std::filesystem::path(file_path).parent_path();
        if (project_config->root_path == "./") {
            project_config->root_path = project_dir.string();
        }
        
        if (!project_config->is_valid()) {
            notify_error("Invalid project configuration in file: " + file_path);
            return false;
        }
        
        // Set as current project
        current_project_ = std::make_unique<ProjectConfig>(*project_config);
        current_project_path_ = std::filesystem::absolute(file_path).string();
        
        notify_project_loaded();
        return true;
    } catch (const std::exception& e) {
        notify_error("Failed to load project: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::save_project() {
    if (!has_project()) {
        notify_error("No project to save");
        return false;
    }
    
    if (current_project_path_.empty()) {
        // Need to save as - generate default filename
        std::string default_path = get_default_project_filename(current_project_->name);
        return save_project_as(default_path);
    }
    
    return save_project_as(current_project_path_);
}

bool ProjectManager::save_project_as(const std::string& file_path) {
    if (!has_project()) {
        notify_error("No project to save");
        return false;
    }
    
    try {
        if (!write_project_file(file_path, *current_project_)) {
            return false;
        }
        
        current_project_path_ = std::filesystem::absolute(file_path).string();
        notify_project_saved();
        return true;
    } catch (const std::exception& e) {
        notify_error("Failed to save project: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::close_project() {
    current_project_.reset();
    current_project_path_.clear();
    return true;
}

const ProjectConfig& ProjectManager::get_current_project() const {
    if (!has_project()) {
        static ProjectConfig empty_project;
        return empty_project;
    }
    return *current_project_;
}

ProjectConfig& ProjectManager::get_current_project_mutable() {
    if (!has_project()) {
        throw std::runtime_error("No current project available");
    }
    return *current_project_;
}

bool ProjectManager::is_project_valid() const {
    return has_project() && current_project_->is_valid();
}

std::vector<std::string> ProjectManager::get_project_validation_errors() const {
    std::vector<std::string> errors;
    
    if (!has_project()) {
        errors.push_back("No project loaded");
        return errors;
    }
    
    const auto& project = *current_project_;
    
    if (project.name.empty()) {
        errors.push_back("Project name is empty");
    }
    
    if (project.root_path.empty()) {
        errors.push_back("Root path is empty");
    } else if (!std::filesystem::exists(project.root_path)) {
        errors.push_back("Root path does not exist: " + project.root_path);
    }
    
    if (project.analysis.source_path.empty()) {
        errors.push_back("Source path is empty");
    } else {
        std::string full_source = project.get_full_source_path();
        if (!std::filesystem::exists(full_source)) {
            errors.push_back("Source path does not exist: " + full_source);
        }
    }
    
    return errors;
}

bool ProjectManager::is_project_file(const std::string& file_path) {
    return std::filesystem::path(file_path).extension() == PROJECT_FILE_EXTENSION;
}

std::string ProjectManager::get_default_project_filename(const std::string& project_name) {
    std::string safe_name = project_name;
    // Replace invalid characters with underscores
    for (char& c : safe_name) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            c = '_';
        }
    }
    return safe_name + PROJECT_FILE_EXTENSION;
}

std::vector<std::string> ProjectManager::find_project_files(const std::string& directory) {
    std::vector<std::string> project_files;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && is_project_file(entry.path().string())) {
                project_files.push_back(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directory for project files: " << e.what() << std::endl;
    }
    
    return project_files;
}

void ProjectManager::update_analysis_config(const ProjectConfig::AnalysisConfig& config) {
    if (has_project()) {
        current_project_->analysis = config;
    }
}

ProjectConfig::AnalysisConfig ProjectManager::get_analysis_config() const {
    if (has_project()) {
        return current_project_->analysis;
    }
    return ProjectConfig::AnalysisConfig{};
}

void ProjectManager::notify_project_loaded() {
    if (on_project_loaded_ && has_project()) {
        on_project_loaded_(*current_project_);
    }
}

void ProjectManager::notify_project_saved() {
    if (on_project_saved_) {
        on_project_saved_(current_project_path_);
    }
}

void ProjectManager::notify_error(const std::string& message) {
    std::cerr << "[PROJECT_MANAGER] Error: " << message << std::endl;
    if (on_error_) {
        on_error_(message);
    }
}

bool ProjectManager::write_project_file(const std::string& file_path, const ProjectConfig& project) {
    try {
        // Serialize project to JSON
        auto json_data = serializer_.serialize(project);
        
        // Add metadata
        nlohmann::json file_data;
        file_data["format_version"] = PROJECT_FILE_VERSION;
        file_data["created_by"] = "Gran Azul Code Quality Analysis Platform";
        file_data["created_at"] = std::chrono::system_clock::now().time_since_epoch().count();
        file_data["project"] = json_data;
        
        // Write to file
        std::ofstream file(file_path);
        if (!file.is_open()) {
            notify_error("Could not open file for writing: " + file_path);
            return false;
        }
        
        file << file_data.dump(2); // Pretty print with 2-space indentation
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        notify_error("Failed to write project file: " + std::string(e.what()));
        return false;
    }
}

std::optional<ProjectConfig> ProjectManager::read_project_file(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            notify_error("Could not open project file: " + file_path);
            return std::nullopt;
        }
        
        nlohmann::json file_data;
        file >> file_data;
        file.close();
        
        // Check format version
        if (file_data.contains("format_version")) {
            std::string version = file_data["format_version"];
            if (version != PROJECT_FILE_VERSION) {
                std::cout << "[PROJECT_MANAGER] Warning: Project file version " << version 
                         << " may not be fully compatible with current version " << PROJECT_FILE_VERSION << std::endl;
            }
        }
        
        // Extract project data
        if (!file_data.contains("project")) {
            notify_error("Invalid project file format: missing project data");
            return std::nullopt;
        }
        
        auto project_result = serializer_.deserialize(file_data["project"]);
        if (!project_result) {
            notify_error("Failed to deserialize project data");
            return std::nullopt;
        }
        
        return *project_result;
    } catch (const std::exception& e) {
        notify_error("Failed to read project file: " + std::string(e.what()));
        return std::nullopt;
    }
}

void ProjectManager::ensure_project_directories(const ProjectConfig& project) {
    try {
        // Create root directory
        std::filesystem::create_directories(project.root_path);
        
        // Create reports directory
        std::filesystem::create_directories(project.get_full_reports_path());
        
        // Create build directory
        std::filesystem::path build_path = std::filesystem::path(project.root_path) / project.analysis.build_dir;
        std::filesystem::create_directories(build_path);
        
        // Create source directories if they don't exist
        for (const auto& source_dir : project.source_directories) {
            std::filesystem::path full_path = std::filesystem::path(project.root_path) / source_dir;
            if (!std::filesystem::exists(full_path)) {
                std::filesystem::create_directories(full_path);
            }
        }
        
        std::cout << "[PROJECT_MANAGER] Project directories ensured" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[PROJECT_MANAGER] Warning: Could not create all project directories: " << e.what() << std::endl;
    }
}

} // namespace gran_azul