#include "log_window_panel.h"
#include <imgui.h>
#include <iostream>

namespace gran_azul::widgets {

LogWindowPanel::LogWindowPanel() 
    : wip::gui::Panel("Command Log"), all_collapsed_(false), force_collapse_state_(false) {
    // Set initial position and size
    set_position(0, 600); // 600px down from top
    set_size(1200, 400);  // Default size
}

void LogWindowPanel::update([[maybe_unused]] float delta_time) {
    // No specific update logic needed
}

void LogWindowPanel::draw_content() {
    render_header();
    render_log_entries();
}

void LogWindowPanel::add_log_entry(const LogEntry& entry) {
    log_entries_.push_back(entry);
}

void LogWindowPanel::add_log_entry(const std::string& command, const wip::utils::process::ProcessResult& result) {
    log_entries_.emplace_back(command, result);
}

void LogWindowPanel::clear_log() {
    log_entries_.clear();
    std::cout << "[LOG_WINDOW_PANEL] Log cleared\n";
}

void LogWindowPanel::render_header() {
    // Header with clear button and collapse all
    if (ImGui::Button("Clear Log")) {
        clear_log();
    }
    ImGui::SameLine();
    if (ImGui::Button(all_collapsed_ ? "Expand All" : "Collapse All")) {
        all_collapsed_ = !all_collapsed_;
        force_collapse_state_ = true; // Force state change on next render
        std::cout << "[LOG_WINDOW_PANEL] " << (all_collapsed_ ? "Collapsed" : "Expanded") << " all log entries\n";
    }
    ImGui::SameLine();
    ImGui::Text("Commands executed: %zu", log_entries_.size());
    
    ImGui::Separator();
}

void LogWindowPanel::render_log_entries() {
    // Log content - scroll to bottom
    ImGuiWindowFlags child_flags = ImGuiWindowFlags_HorizontalScrollbar;
    if (ImGui::BeginChild("LogContent", ImVec2(0, 0), false, child_flags)) {
        // Iterate in reverse order to show latest commands at the top
        for (int idx = static_cast<int>(log_entries_.size()) - 1; idx >= 0; --idx) {
            size_t i = static_cast<size_t>(idx);
            size_t display_number = log_entries_.size() - i;
            render_log_entry(log_entries_[i], i, display_number);
        }
        
        // Reset the force flag after processing all headers
        force_collapse_state_ = false;
        
        // Auto-scroll to bottom when new entries are added
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
}

void LogWindowPanel::render_log_entry(const LogEntry& entry, size_t index, size_t display_number) {
    ImGui::PushID(static_cast<int>(index));
    
    // Color-code based on success
    ImVec4 color = entry.success ? ImVec4(0.0f, 0.8f, 0.0f, 1.0f) : ImVec4(0.8f, 0.0f, 0.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    
    // Force the collapse state if the button was pressed
    if (force_collapse_state_) {
        ImGui::SetNextItemOpen(!all_collapsed_);
    }
    
    bool is_open = ImGui::CollapsingHeader(("Command " + std::to_string(display_number) + " - " + entry.timestamp).c_str());
    
    if (is_open) {
        ImGui::PopStyleColor(); // Reset color for content
        
        render_command_section(entry, index);
        render_output_section(entry, index);
    } else {
        ImGui::PopStyleColor(); // Reset color if header is collapsed
    }
    
    ImGui::PopID();
    ImGui::Spacing();
}

void LogWindowPanel::render_command_section(const LogEntry& entry, size_t index) {
    // Display command in a scrollable, copyable area
    ImGui::Text("Command:");
    ImGui::SameLine();
    if (ImGui::Button(("Copy Command##cmd_" + std::to_string(index)).c_str())) {
        wip::gui::widgets::SelectableTextWidget::copy_text_to_clipboard(entry.command);
    }
    
    ImGui::BeginChild(("CommandText_" + std::to_string(index)).c_str(), ImVec2(0, 60), true);
    wip::gui::widgets::SelectableTextWidget::render_selectable_text_lines(entry.command, "command_" + std::to_string(index));
    ImGui::EndChild();
    
    ImGui::Text("Exit Code: %d", entry.exit_code);
    ImGui::Text("Duration: %ld ms", entry.duration.count());
    ImGui::Text("Success: %s", entry.success ? "Yes" : "No");
    
    ImGui::Separator();
}

void LogWindowPanel::render_output_section(const LogEntry& entry, size_t index) {
    // Display stdout
    if (!entry.stdout_output.empty()) {
        ImGui::Text("Standard Output:");
        
        // Add copy button for stdout
        ImGui::SameLine();
        if (ImGui::Button(("Copy Stdout##stdout_" + std::to_string(index)).c_str())) {
            wip::gui::widgets::SelectableTextWidget::copy_text_to_clipboard(entry.stdout_output);
        }
        
        // Display stdout in a selectable child window
        ImGui::BeginChild(("StdoutText_" + std::to_string(index)).c_str(), ImVec2(0, 100), true);
        wip::gui::widgets::SelectableTextWidget::render_selectable_text_lines(entry.stdout_output, "stdout_" + std::to_string(index));
        ImGui::EndChild();
    }
    
    // Display stderr
    if (!entry.stderr_output.empty()) {
        if (!entry.stdout_output.empty()) {
            ImGui::Spacing();
        }
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 0.0f, 1.0f)); // Orange for stderr
        ImGui::Text("Standard Error:");
        ImGui::PopStyleColor();
        
        // Add copy button for stderr
        ImGui::SameLine();
        if (ImGui::Button(("Copy Stderr##stderr_" + std::to_string(index)).c_str())) {
            wip::gui::widgets::SelectableTextWidget::copy_text_to_clipboard(entry.stderr_output);
        }
        
        // Display stderr in a selectable child window
        ImGui::BeginChild(("StderrText_" + std::to_string(index)).c_str(), ImVec2(0, 100), true);
        wip::gui::widgets::SelectableTextWidget::render_selectable_text_lines(entry.stderr_output, "stderr_" + std::to_string(index));
        ImGui::EndChild();
    }
    
    // If both are empty
    if (entry.stdout_output.empty() && entry.stderr_output.empty()) {
        ImGui::Text("No output");
    }
}

} // namespace gran_azul::widgets
