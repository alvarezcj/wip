#include "project_startup_modal.h"
#include <filesystem>
#include <iostream>
#include <cstring>

namespace gran_azul {
namespace widgets {

ProjectStartupModal::ProjectStartupModal() {
    // Initialize project path to user's home directory
    std::string home_dir = std::getenv("HOME") ? std::getenv("HOME") : ".";
    strncpy(project_path_, home_dir.c_str(), sizeof(project_path_) - 1);
    project_path_[sizeof(project_path_) - 1] = '\0';
    
    // Initialize source path to current directory
    strncpy(source_path_, ".", sizeof(source_path_) - 1);
    source_path_[sizeof(source_path_) - 1] = '\0';
}

bool ProjectStartupModal::render() {
    if (!show_modal_) {
        return false;
    }
    
    // Create a full-screen overlay window that acts like a modal
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    // Window flags for full-screen overlay
    ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoTitleBar | 
                                     ImGuiWindowFlags_NoResize | 
                                     ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoBringToFrontOnFocus |
                                     ImGuiWindowFlags_NoNavFocus |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    bool open = true;
    if (ImGui::Begin("##ProjectStartupOverlay", &open, overlay_flags)) {
        ImGui::PopStyleVar(3);
        
        // Draw semi-transparent background
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(viewport->Pos, 
                                ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
                                IM_COL32(0, 0, 0, 180)); // Semi-transparent black
        
        // Center the modal content
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
        
        // Modal content window
        ImGuiWindowFlags modal_flags = ImGuiWindowFlags_NoResize | 
                                       ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_AlwaysAutoResize;
        
        if (ImGui::Begin("Gran Azul - Project Setup", nullptr, modal_flags)) {
            // Render appropriate content based on current state
            if (show_new_project_form_) {
                render_new_project_form();
            } else if (show_load_project_dialog_) {
                render_load_project_dialog();
            } else {
                render_welcome_screen();
            }
            ImGui::End();
        }
        
        ImGui::End();
    } else {
        ImGui::PopStyleVar(3);
    }
    
    // Render error popup if needed
    render_error_popup();
    
    return show_modal_;
}

void ProjectStartupModal::render_welcome_screen() {
    // Gran Azul logo/title
    // Just use regular text for now - could be enhanced with larger font later
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Welcome to Gran Azul").x) * 0.5f);
    ImGui::Text("Welcome to Gran Azul");
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Code Quality Analysis Platform").x) * 0.5f);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Code Quality Analysis Platform");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextWrapped("To get started, you need to create a new project or load an existing one. "
                      "Projects contain your analysis configuration and results.");
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Center the buttons
    float button_width = 200.0f;
    float spacing = 20.0f;
    float total_width = button_width * 2 + spacing;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - total_width) * 0.5f);
    
    // New project button
    if (ImGui::Button("Create New Project", ImVec2(button_width, 40))) {
        show_new_project_form_ = true;
        reset_form_data();
    }
    
    ImGui::SameLine(0, spacing);
    
    // Load project button
    if (ImGui::Button("Load Existing Project", ImVec2(button_width, 40))) {
        show_load_project_dialog_ = true;
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    
    // Exit button (bottom right)
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 80);
    if (ImGui::Button("Exit", ImVec2(70, 0))) {
        if (on_exit_) {
            on_exit_();
        }
        show_modal_ = false;
    }
}

void ProjectStartupModal::render_new_project_form() {
    ImGui::Text("Create New Project");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Project name
    ImGui::Text("Project Name:");
    ImGui::InputText("##project_name", project_name_, sizeof(project_name_));
    
    ImGui::Spacing();
    
    // Project location
    ImGui::Text("Project Location:");
    ImGui::InputText("##project_path", project_path_, sizeof(project_path_));
    ImGui::SameLine();
    if (ImGui::Button("Browse##project")) {
        browse_project_path();
    }
    
    ImGui::Spacing();
    
    // Source code path
    ImGui::Text("Source Code Path:");
    ImGui::InputText("##source_path", source_path_, sizeof(source_path_));
    ImGui::SameLine();
    if (ImGui::Button("Browse##source")) {
        browse_source_path();
    }
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                      "The project file will be created as: %s/%s.granazul", 
                      project_path_, project_name_);
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Buttons
    if (ImGui::Button("Create Project", ImVec2(120, 0))) {
        if (validate_new_project_data()) {
            if (on_new_project_) {
                on_new_project_(std::string(project_name_), 
                              std::string(project_path_), 
                              std::string(source_path_));
            }
            show_modal_ = false;
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        show_new_project_form_ = false;
    }
}

void ProjectStartupModal::render_load_project_dialog() {
    ImGui::Text("Load Existing Project");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextWrapped("Click the button below to browse for a Gran Azul project file (.granazul):");
    
    ImGui::Spacing();
    
    if (ImGui::Button("Browse for Project File", ImVec2(200, 40))) {
        nfdchar_t* outPath = nullptr;
        nfdfilteritem_t filterItem[1] = { { "Gran Azul Project", "granazul" } };
        
        nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, nullptr);
        if (result == NFD_OKAY) {
            if (on_load_project_) {
                on_load_project_(std::string(outPath));
            }
            NFD_FreePath(outPath);
            show_modal_ = false;
        } else if (result != NFD_CANCEL) {
            show_error("Failed to open file dialog");
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Back button
    if (ImGui::Button("Back", ImVec2(120, 0))) {
        show_load_project_dialog_ = false;
    }
}

void ProjectStartupModal::render_error_popup() {
    if (!error_message_.empty()) {
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        
        if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error");
            ImGui::Separator();
            ImGui::Text("%s", error_message_.c_str());
            ImGui::Spacing();
            
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                error_message_.clear();
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
        
        if (!ImGui::IsPopupOpen("Error")) {
            ImGui::OpenPopup("Error");
        }
    }
}

bool ProjectStartupModal::validate_new_project_data() {
    // Check project name
    if (strlen(project_name_) == 0) {
        show_error("Project name cannot be empty");
        return false;
    }
    
    // Check project path
    if (strlen(project_path_) == 0) {
        show_error("Project location cannot be empty");
        return false;
    }
    
    // Check source path
    if (strlen(source_path_) == 0) {
        show_error("Source code path cannot be empty");
        return false;
    }
    
    // Validate paths exist
    std::filesystem::path project_dir(project_path_);
    if (!std::filesystem::exists(project_dir)) {
        show_error("Project location directory does not exist");
        return false;
    }
    
    std::filesystem::path source_dir(source_path_);
    if (!std::filesystem::exists(source_dir)) {
        show_error("Source code path does not exist");
        return false;
    }
    
    // Check if project file would already exist
    std::filesystem::path project_file = project_dir / (std::string(project_name_) + ".granazul");
    if (std::filesystem::exists(project_file)) {
        show_error("A project with this name already exists in the selected location");
        return false;
    }
    
    return true;
}

void ProjectStartupModal::reset_form_data() {
    strncpy(project_name_, "My Code Analysis Project", sizeof(project_name_) - 1);
    project_name_[sizeof(project_name_) - 1] = '\0';
    
    // Keep existing paths as they're likely still relevant
}

void ProjectStartupModal::browse_project_path() {
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_PickFolder(&outPath, project_path_);
    
    if (result == NFD_OKAY) {
        strncpy(project_path_, outPath, sizeof(project_path_) - 1);
        project_path_[sizeof(project_path_) - 1] = '\0';
        NFD_FreePath(outPath);
    } else if (result != NFD_CANCEL) {
        show_error("Failed to open directory dialog");
    }
}

void ProjectStartupModal::browse_source_path() {
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_PickFolder(&outPath, source_path_);
    
    if (result == NFD_OKAY) {
        strncpy(source_path_, outPath, sizeof(source_path_) - 1);
        source_path_[sizeof(source_path_) - 1] = '\0';
        NFD_FreePath(outPath);
    } else if (result != NFD_CANCEL) {
        show_error("Failed to open directory dialog");
    }
}

bool ProjectStartupModal::create_project_directory(const std::string& path) {
    try {
        std::filesystem::create_directories(path);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to create project directory: " << e.what() << std::endl;
        return false;
    }
}

} // namespace widgets
} // namespace gran_azul