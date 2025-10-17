#include "path_selector_widget.h"
#include <imgui.h>
#include <nfd.h>
#include <filesystem>
#include <iostream>

namespace gran_azul::widgets {

PathSelectorWidget::PathSelectorWidget(const std::string& label, PathType type, float label_width)
    : label_(label), path_type_(type), label_width_(label_width), button_width_(30.0f) {
}

void PathSelectorWidget::draw() {
    draw_inline();
}

bool PathSelectorWidget::draw_inline() {
    bool path_changed = false;
    
    // Label
    ImGui::Text("%s", label_.c_str());
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (label_width_ - ImGui::CalcTextSize(label_.c_str()).x));
    
    // Input text field
    float available_width = ImGui::GetContentRegionAvail().x - button_width_ - ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(available_width);
    
    char path_buffer[512];
    strncpy(path_buffer, current_path_.c_str(), sizeof(path_buffer) - 1);
    path_buffer[sizeof(path_buffer) - 1] = '\0';
    
    std::string input_id = "##path_input_" + label_;
    if (ImGui::InputText(input_id.c_str(), path_buffer, sizeof(path_buffer))) {
        current_path_ = path_buffer;
        if (on_path_selected_) {
            on_path_selected_(current_path_);
        }
        path_changed = true;
    }
    
    // Browse button
    ImGui::SameLine();
    std::string button_id = "...##browse_" + label_;
    if (ImGui::Button(button_id.c_str(), ImVec2(button_width_, 0))) {
        open_dialog();
        path_changed = true;
    }
    
    return path_changed;
}

void PathSelectorWidget::open_dialog() {
    nfdchar_t* out_path = nullptr;
    nfdresult_t result = NFD_CANCEL;
    
    if (path_type_ == PathType::Folder) {
        // Folder selection
        result = NFD_PickFolder(&out_path, nullptr);
    } else {
        // File selection
        if (filters_.empty()) {
            // No filters - open any file
            result = NFD_OpenDialog(&out_path, nullptr, 0, nullptr);
        } else {
            // Convert our filters to NFD format
            std::vector<nfdfilteritem_t> nfd_filters;
            for (const auto& filter : filters_) {
                nfdfilteritem_t nfd_filter;
                nfd_filter.name = filter.name.c_str();
                nfd_filter.spec = filter.extension.c_str();
                nfd_filters.push_back(nfd_filter);
            }
            result = NFD_OpenDialog(&out_path, nfd_filters.data(), nfd_filters.size(), nullptr);
        }
    }
    
    if (result == NFD_OKAY) {
        current_path_ = out_path;
        
        // Notify callback
        if (on_path_selected_) {
            on_path_selected_(current_path_);
        }
        
        std::cout << "[PATH_SELECTOR] Selected: " << current_path_ << std::endl;
        
        // Remember to free the path memory
        NFD_FreePath(out_path);
    } else if (result == NFD_CANCEL) {
        std::cout << "[PATH_SELECTOR] Selection cancelled\n";
    } else {
        std::cout << "[PATH_SELECTOR] Error: " << NFD_GetError() << "\n";
    }
}

std::string PathSelectorWidget::get_relative_path(const std::string& absolute_path, const std::string& base_path) {
    try {
        std::filesystem::path abs_path = std::filesystem::absolute(absolute_path);
        std::filesystem::path base = std::filesystem::absolute(base_path);
        
        std::filesystem::path relative = std::filesystem::relative(abs_path, base);
        return relative.string();
    } catch (const std::exception& e) {
        std::cout << "[PATH_SELECTOR] Error calculating relative path: " << e.what() << std::endl;
        return absolute_path;
    }
}

} // namespace gran_azul::widgets