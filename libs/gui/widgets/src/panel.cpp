#include "panel.h"
#include <imgui.h>
#include <algorithm>

namespace wip::gui {

Panel::Panel(const std::string& title, const std::string& id) 
    : Widget(id.empty() ? ("panel_" + title) : id), title_(title) {
}

void Panel::update(float delta_time) {
    // Update base widget
    Widget::update(delta_time);
    
    // Update positioning and sizing if needed
    if (needs_position_update_) {
        update_position();
        needs_position_update_ = false;
    }
    
    if (needs_size_update_) {
        update_size();  
        needs_size_update_ = false;
    }
}

void Panel::draw() {
    if (!is_visible() || !open_) {
        return;
    }
    
    on_before_draw();
    
    // Handle modal panels
    if (modal_) {
        ImGui::OpenPopup(get_id().c_str());
        if (ImGui::BeginPopupModal(title_.c_str(), &open_, window_flags_)) {
            draw_content();
            on_after_draw();
            ImGui::EndPopup();
        }
    } else {
        // Regular window
        if (ImGui::Begin(title_.c_str(), &open_, window_flags_)) {
            draw_content();
            on_after_draw();
        }
        ImGui::End();
    }
    
    first_frame_ = false;
}

void Panel::set_position(float x, float y) {
    position_x_ = x;
    position_y_ = y;
    position_mode_ = PositionMode::Manual;
    needs_position_update_ = true;
}

void Panel::set_position_offset(float offset_x, float offset_y) {
    position_offset_x_ = offset_x;
    position_offset_y_ = offset_y;
    needs_position_update_ = true;
}

void Panel::set_size(float width, float height) {
    size_width_ = width;
    size_height_ = height;
    size_mode_ = SizeMode::Manual;
    needs_size_update_ = true;
}

void Panel::set_size_percentage(float width_percent, float height_percent) {
    size_width_percent_ = std::clamp(width_percent, 0.0f, 1.0f);
    size_height_percent_ = std::clamp(height_percent, 0.0f, 1.0f);
    size_mode_ = SizeMode::Percentage;
    needs_size_update_ = true;
}

void Panel::set_auto_resize(bool auto_resize) {
    if (auto_resize) {
        add_window_flag(ImGuiWindowFlags_AlwaysAutoResize);
        size_mode_ = SizeMode::Auto;
    } else {
        remove_window_flag(ImGuiWindowFlags_AlwaysAutoResize);
        if (size_mode_ == SizeMode::Auto) {
            size_mode_ = SizeMode::Manual;
        }
    }
    needs_size_update_ = true;
}

void Panel::update_position() {
    if (!first_frame_ && position_mode_ == PositionMode::Manual) {
        // For manual mode, only set position on first frame or when explicitly changed
        return;
    }
    
    ImVec2 viewport_size = get_viewport_size();
    ImVec2 pos;
    
    switch (position_mode_) {
        case PositionMode::Manual:
            pos = ImVec2(position_x_, position_y_);
            break;
            
        case PositionMode::Centered:
            pos = ImVec2(viewport_size.x * 0.5f, viewport_size.y * 0.5f);
            break;
            
        case PositionMode::TopLeft:
            pos = ImVec2(10.0f, 10.0f);
            break;
            
        case PositionMode::TopRight:
            pos = ImVec2(viewport_size.x - 10.0f, 10.0f);
            break;
            
        case PositionMode::BottomLeft:
            pos = ImVec2(10.0f, viewport_size.y - 10.0f);
            break;
            
        case PositionMode::BottomRight:
            pos = ImVec2(viewport_size.x - 10.0f, viewport_size.y - 10.0f);
            break;
    }
    
    // Apply offset
    pos.x += position_offset_x_;
    pos.y += position_offset_y_;
    
    // Set position for next frame
    ImGui::SetNextWindowPos(pos, first_frame_ ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
}

void Panel::update_size() {
    if (!first_frame_ && size_mode_ == SizeMode::Manual) {
        // For manual mode, only set size on first frame or when explicitly changed
        return;
    }
    
    ImVec2 size;
    ImVec2 viewport_size = get_viewport_size();
    
    switch (size_mode_) {
        case SizeMode::Manual:
            size = ImVec2(size_width_, size_height_);
            break;
            
        case SizeMode::Auto:
            // Let ImGui handle auto-sizing, don't set size
            return;
            
        case SizeMode::FullScreen:
            size = viewport_size;
            break;
            
        case SizeMode::Percentage:
            size = ImVec2(viewport_size.x * size_width_percent_, 
                         viewport_size.y * size_height_percent_);
            break;
    }
    
    // Set size for next frame
    ImGui::SetNextWindowSize(size, first_frame_ ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
}

ImVec2 Panel::get_viewport_size() const {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    return viewport ? viewport->WorkSize : ImVec2(800, 600); // Fallback size
}

} // namespace wip::gui