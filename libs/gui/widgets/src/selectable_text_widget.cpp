#include "selectable_text_widget.h"
#include <imgui.h>
#include <iostream>

namespace wip::gui::widgets {

SelectableTextWidget::SelectableTextWidget(const std::string& text, const std::string& unique_id)
    : text_(text), unique_id_(unique_id) {
}

void SelectableTextWidget::update([[maybe_unused]] float delta_time) {
    // No specific update logic needed for this widget
}

void SelectableTextWidget::draw() {
    render_text_lines();
}

void SelectableTextWidget::render_text_lines() {
    render_selectable_text_lines(text_, unique_id_, on_text_copied_);
}

void SelectableTextWidget::render_selectable_text_lines(const std::string& text, const std::string& base_id, TextCopyCallback copy_callback) {
    if (text.empty()) return;
    
    std::string text_copy = text;
    std::string line;
    size_t line_num = 0;
    size_t start = 0;
    size_t end = 0;
    
    while ((end = text_copy.find('\n', start)) != std::string::npos) {
        line = text_copy.substr(start, end - start);
        std::string line_id = base_id + "_line_" + std::to_string(line_num);
        
        if (ImGui::Selectable((line + "##" + line_id).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (ImGui::IsMouseDoubleClicked(0)) {
                copy_text_to_clipboard(line);
                if (copy_callback) {
                    copy_callback(line);
                }
            }
        }
        start = end + 1;
        ++line_num;
    }
    
    // Handle the last line (or the only line if no newlines)
    if (start < text_copy.length()) {
        line = text_copy.substr(start);
        std::string line_id = base_id + "_line_" + std::to_string(line_num);
        
        if (ImGui::Selectable((line + "##" + line_id).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (ImGui::IsMouseDoubleClicked(0)) {
                copy_text_to_clipboard(line);
                if (copy_callback) {
                    copy_callback(line);
                }
            }
        }
    }
}

void SelectableTextWidget::copy_text_to_clipboard(const std::string& text) {
    ImGui::SetClipboardText(text.c_str());
    std::cout << "[SELECTABLE_TEXT_WIDGET] Text copied to clipboard\n";
}

} // namespace wip::gui::widgets