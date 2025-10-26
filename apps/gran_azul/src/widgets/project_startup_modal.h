#pragma once

#include <imgui.h>
#include <string>
#include <functional>
#include <nfd.h>

namespace gran_azul {
namespace widgets {

/**
 * @brief Modal dialog for project creation and loading at application startup
 * 
 * This modal forces the user to either create a new project or load an existing one
 * before they can access the main application functionality.
 */
class ProjectStartupModal {
public:
    /**
     * @brief Callback for when a new project should be created
     * @param project_name Name of the new project
     * @param project_path Directory where project should be created
     * @param source_path Path to source code to analyze
     */
    using NewProjectCallback = std::function<void(const std::string& project_name, 
                                                  const std::string& project_path,
                                                  const std::string& source_path)>;
    
    /**
     * @brief Callback for when an existing project should be loaded
     * @param project_file Path to the .granazul project file
     */
    using LoadProjectCallback = std::function<void(const std::string& project_file)>;
    
    /**
     * @brief Callback for when the application should exit
     */
    using ExitCallback = std::function<void()>;

private:
    bool show_modal_ = true;
    bool show_new_project_form_ = false;
    
    // New project form data
    char project_name_[256] = "My Code Analysis Project";
    char project_path_[512] = "";
    char source_path_[512] = "";
    
    // Error state
    std::string error_message_;
    
    // Callbacks
    NewProjectCallback on_new_project_;
    LoadProjectCallback on_load_project_;
    ExitCallback on_exit_;
    
    // Helper methods
    void render_welcome_screen();
    void render_new_project_form();
    void render_error_popup();
    
    bool validate_new_project_data();
    void reset_form_data();
    void browse_project_path();
    void browse_source_path();

public:
    ProjectStartupModal();
    ~ProjectStartupModal() = default;
    
    /**
     * @brief Render the modal (call this every frame while active)
     * @return true if modal is still active, false if closed
     */
    bool render();
    
    /**
     * @brief Check if the modal is currently visible
     */
    bool is_visible() const { return show_modal_; }
    
    /**
     * @brief Force close the modal (project successfully created/loaded)
     */
    void close() { show_modal_ = false; }
    
    /**
     * @brief Show an error message in the modal
     */
    void show_error(const std::string& message) { error_message_ = message; }
    
    // Callback setters
    void set_new_project_callback(NewProjectCallback callback) { on_new_project_ = std::move(callback); }
    void set_load_project_callback(LoadProjectCallback callback) { on_load_project_ = std::move(callback); }
    void set_exit_callback(ExitCallback callback) { on_exit_ = std::move(callback); }
    
    // Helper for creating default project directory structure
    static bool create_project_directory(const std::string& path);
};

} // namespace widgets
} // namespace gran_azul