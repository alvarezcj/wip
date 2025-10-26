#include "progress_dialog.h"
#include <imgui.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace gran_azul::widgets {

ProgressDialog::ProgressDialog(const std::string& title)
    : title_(title)
    , current_status_("Ready")
    , progress_value_(0.0f)
    , is_visible_(false)
    , can_cancel_(true)
    , is_completed_(false)
    , auto_scroll_output_(true)
    , auto_close_on_completion_(false)
    , popup_opened_(false)
    , scroll_to_bottom_(false)
{
    memset(output_filter_, 0, sizeof(output_filter_));
}

void ProgressDialog::update(float delta_time) {
    // Update timing
    last_update_ = std::chrono::steady_clock::now();
}

void ProgressDialog::draw() {
    if (!is_visible_) {
        popup_opened_ = false;  // Reset when not visible
        return;
    }
    
    // Open popup modal when first becoming visible
    if (!popup_opened_) {
        ImGui::OpenPopup(title_.c_str());
        popup_opened_ = true;
    }
    
    // Center the modal window
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_Always);
    
    // Modal popup flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | 
                            ImGuiWindowFlags_NoResize | 
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoSavedSettings;
    
    bool window_open = true;
    if (ImGui::BeginPopupModal(title_.c_str(), (can_cancel_ || is_completed_) ? &window_open : nullptr, flags)) {
        
        // This window is now acting as a modal - no need for capture functions
        
        
        render_progress_bar();
        render_status_text();
        
        ImGui::Separator();
        
        render_output_section();
        
        ImGui::Separator();
        
        render_control_buttons();
        
        ImGui::EndPopup();
    }
    
    // Handle window close (user clicked X or escape)
    if (!window_open && (can_cancel_ || is_completed_)) {
        popup_opened_ = false;
        hide();
    }
}

void ProgressDialog::show(const std::string& initial_status) {
    is_visible_ = true;
    is_completed_ = false;
    progress_value_ = 0.0f;
    current_status_ = initial_status;
    output_text_.clear();
    start_time_ = std::chrono::steady_clock::now();
    last_update_ = start_time_;
    popup_opened_ = false;  // Reset popup state
    scroll_to_bottom_ = false;  // Reset scroll flag
    
    std::cout << "[PROGRESS_DIALOG] show() called with status: " << initial_status << std::endl;
}

void ProgressDialog::hide() {
    is_visible_ = false;
    popup_opened_ = false;  // Reset popup state
}

void ProgressDialog::set_progress(float progress, const std::string& status) {
    progress_value_ = std::clamp(progress, 0.0f, 1.0f);
    if (!status.empty()) {
        current_status_ = status;
    }
    last_update_ = std::chrono::steady_clock::now();
}

void ProgressDialog::add_output_line(const std::string& line) {
    std::cout << "[PROGRESS_DIALOG] add_output_line called with: '" << line << "'" << std::endl;
    
    if (!output_text_.empty()) {
        output_text_ += "\n";
    }
    output_text_ += line;
    
    // Set flag to scroll to bottom on next render
    if (auto_scroll_output_) {
        scroll_to_bottom_ = true;
    }
    
    // Limit output size to prevent memory issues
    const size_t MAX_OUTPUT_SIZE = 50000; // ~50KB
    if (output_text_.size() > MAX_OUTPUT_SIZE) {
        // Remove oldest lines
        size_t newline_pos = output_text_.find('\n', output_text_.size() - MAX_OUTPUT_SIZE + 1000);
        if (newline_pos != std::string::npos) {
            output_text_ = output_text_.substr(newline_pos + 1);
        }
    }
}

void ProgressDialog::set_completed(bool success, const std::string& final_message) {
    std::cout << "[PROGRESS_DIALOG] Setting dialog as completed, success=" << success << std::endl;
    is_completed_ = true;
    progress_value_ = 1.0f;
    can_cancel_ = true; // Allow closing when completed
    completion_time_ = std::chrono::steady_clock::now();
    
    if (!final_message.empty()) {
        current_status_ = final_message;
    } else {
        current_status_ = success ? "Completed successfully" : "Completed with errors";
    }
    std::cout << "[PROGRESS_DIALOG] Dialog state: visible=" << is_visible_ << ", completed=" << is_completed_ << std::endl;
}

void ProgressDialog::clear_output() {
    output_text_.clear();
}

void ProgressDialog::render_progress_bar() {
    // Progress bar with percentage
    ImGui::Text("Progress:");
    ImGui::SameLine();
    
    // Progress bar
    char progress_text[32];
    snprintf(progress_text, sizeof(progress_text), "%.1f%%", progress_value_ * 100.0f);
    ImGui::ProgressBar(progress_value_, ImVec2(-1.0f, 0.0f), progress_text);
    
    // Timing information
    ImGui::Text("Elapsed: %s", format_elapsed_time().c_str());
    
    if (progress_value_ > 0.0f && progress_value_ < 1.0f) {
        ImGui::SameLine();
        ImGui::Text(" | ETA: %s", format_eta(progress_value_).c_str());
    }
}

void ProgressDialog::render_status_text() {
    ImGui::Text("Status: %s", current_status_.c_str());
}

void ProgressDialog::render_output_section() {
    ImGui::Text("Output:");
    ImGui::SameLine();
    
    // Filter input
    ImGui::PushItemWidth(200);
    ImGui::InputTextWithHint("##output_filter", "Filter output...", output_filter_, sizeof(output_filter_));
    ImGui::PopItemWidth();
    
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_output_);
    
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        clear_output();
    }
    
    // Output text area
    ImVec2 output_size = ImVec2(-1, ImGui::GetContentRegionAvail().y - 60); // Reserve space for buttons
    if (ImGui::BeginChild("OutputText", output_size, true, ImGuiWindowFlags_HorizontalScrollbar)) {
        if (!output_text_.empty()) {
            std::string filter_str = output_filter_;
            std::transform(filter_str.begin(), filter_str.end(), filter_str.begin(), ::tolower);
            
            if (filter_str.empty()) {
                // No filter - show all text
                ImGui::TextUnformatted(output_text_.c_str());
            } else {
                // Apply filter - show matching lines
                std::istringstream iss(output_text_);
                std::string line;
                while (std::getline(iss, line)) {
                    std::string line_lower = line;
                    std::transform(line_lower.begin(), line_lower.end(), line_lower.begin(), ::tolower);
                    
                    if (line_lower.find(filter_str) != std::string::npos) {
                        ImGui::TextUnformatted(line.c_str());
                    }
                }
            }
            
            // Auto-scroll to bottom when new content is added or user is already at bottom
            if (auto_scroll_output_) {
                if (scroll_to_bottom_ || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                    ImGui::SetScrollHereY(1.0f);
                    scroll_to_bottom_ = false;  // Reset flag after scrolling
                }
            }
        }
    }
    ImGui::EndChild();
}

void ProgressDialog::render_control_buttons() {
    // Center buttons
    float button_width = 80.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float total_width = is_completed_ ? button_width : (button_width * 2 + spacing);
    float offset = (ImGui::GetContentRegionAvail().x - total_width) * 0.5f;
    
    if (offset > 0) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }
    
    if (is_completed_) {
        if (ImGui::Button("Close", ImVec2(button_width, 0))) {
            hide();
        }
    } else {
        // Cancel button
        bool can_cancel_now = can_cancel_ && !is_completed_;
        if (!can_cancel_now) {
            ImGui::BeginDisabled();
        }
        
        if (ImGui::Button("Cancel", ImVec2(button_width, 0))) {
            if (on_cancel_) {
                on_cancel_();
            }
            hide();
        }
        
        if (!can_cancel_now) {
            ImGui::EndDisabled();
        }
        
        ImGui::SameLine();
        
        // Hide button (minimize to background)
        if (ImGui::Button("Hide", ImVec2(button_width, 0))) {
            hide();
        }
    }
}

std::string ProgressDialog::format_elapsed_time() const {
    auto end_time = is_completed_ ? completion_time_ : std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time_);
    
    int hours = elapsed.count() / 3600;
    int minutes = (elapsed.count() % 3600) / 60;
    int seconds = elapsed.count() % 60;
    
    std::ostringstream oss;
    if (hours > 0) {
        oss << std::setfill('0') << std::setw(2) << hours << ":";
    }
    oss << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << seconds;
    
    return oss.str();
}

std::string ProgressDialog::format_eta(float progress) const {
    if (progress <= 0.0f) {
        return "Unknown";
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    
    // Estimate total time based on current progress
    double total_estimated_seconds = elapsed.count() / progress;
    int remaining_seconds = static_cast<int>(total_estimated_seconds - elapsed.count());
    
    if (remaining_seconds < 0) {
        remaining_seconds = 0;
    }
    
    int hours = remaining_seconds / 3600;
    int minutes = (remaining_seconds % 3600) / 60;
    int seconds = remaining_seconds % 60;
    
    std::ostringstream oss;
    if (hours > 0) {
        oss << std::setfill('0') << std::setw(2) << hours << ":";
    }
    oss << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << seconds;
    
    return oss.str();
}

} // namespace gran_azul::widgets