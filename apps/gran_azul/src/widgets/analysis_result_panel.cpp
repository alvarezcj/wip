#include "analysis_result_panel.h"
#include <imgui.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace gran_azul::widgets {

AnalysisResultPanel::AnalysisResultPanel(const std::string& title) 
    : Panel(title) {
    // Initialize with reasonable default size
    set_size(800, 600);
}

void AnalysisResultPanel::update(float delta_time) {
    Panel::update(delta_time);
}

void AnalysisResultPanel::draw_content() {
    if (result_.issues.empty() && result_.analysis_successful) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✓ No issues found!");
        ImGui::Text("Analysis completed successfully with no problems detected.");
        return;
    }
    
    if (!result_.analysis_successful && !result_.error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "✗ Analysis failed");
        ImGui::TextWrapped("Error: %s", result_.error_message.c_str());
        return;
    }
    
    if (result_.issues.empty()) {
        ImGui::Text("No analysis results available.");
        ImGui::Text("Run cppcheck analysis to see results here.");
        return;
    }
    
    render_summary();
    ImGui::Separator();
    render_filters();
    ImGui::Separator();
    render_issues_table();
}

void AnalysisResultPanel::render_summary() {
    ImGui::Text("Analysis Summary");
    
    if (!result_.timestamp.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(%s)", result_.timestamp.c_str());
    }
    
    ImGui::Columns(6, "SummaryColumns", false);
    
    // Error count
    auto error_count = result_.count_by_severity(IssueSeverity::ERROR);
    AnalysisIssue error_issue;
    error_issue.severity = IssueSeverity::ERROR;
    auto error_color = error_issue.severity_color();
    ImGui::TextColored(ImVec4(error_color.r, error_color.g, error_color.b, error_color.a), 
                      "Errors: %zu", error_count);
    
    ImGui::NextColumn();
    
    // Warning count
    auto warning_count = result_.count_by_severity(IssueSeverity::WARNING);
    AnalysisIssue warning_issue;
    warning_issue.severity = IssueSeverity::WARNING;
    auto warning_color_struct = warning_issue.severity_color();
    auto warning_color = ImVec4(warning_color_struct.r, warning_color_struct.g, warning_color_struct.b, warning_color_struct.a);
    ImGui::TextColored(warning_color, "Warnings: %zu", warning_count);
    
    ImGui::NextColumn();
    
    // Style count  
    auto style_count = result_.count_by_severity(IssueSeverity::STYLE);
    ImGui::Text("Style: %zu", style_count);
    
    ImGui::NextColumn();
    
    // Performance count
    auto perf_count = result_.count_by_severity(IssueSeverity::PERFORMANCE);
    ImGui::Text("Performance: %zu", perf_count);
    
    ImGui::NextColumn();
    
    // Total issues
    ImGui::Text("Total Issues: %zu", result_.issues.size());
    
    ImGui::NextColumn();
    
    // Files analyzed
    ImGui::Text("Files: %d", result_.total_files_analyzed);
    
    ImGui::Columns(1);
}

void AnalysisResultPanel::render_filters() {
    ImGui::Text("Filters");
    
    // Text filter
    ImGui::PushItemWidth(200);
    ImGui::InputText("##filter", filter_text_, sizeof(filter_text_));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Text("Search");
    
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(20, 0)); // Spacing
    ImGui::SameLine();
    
    // Severity filters
    ImGui::Text("Show:");
    ImGui::SameLine();
    ImGui::Checkbox("Errors##filter", &show_errors_);
    ImGui::SameLine();
    ImGui::Checkbox("Warnings##filter", &show_warnings_);
    ImGui::SameLine();
    ImGui::Checkbox("Style##filter", &show_style_);
    ImGui::SameLine();
    ImGui::Checkbox("Performance##filter", &show_performance_);
    ImGui::SameLine();
    ImGui::Checkbox("Portability##filter", &show_portability_);
    ImGui::SameLine();
    ImGui::Checkbox("Info##filter", &show_information_);
    
    // New line for false positive filter
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(20, 0)); // Spacing
    ImGui::SameLine();
    ImGui::Checkbox("False Positives##filter", &show_false_positives_);
}

void AnalysisResultPanel::render_issues_table() {
    auto filtered_issues = get_filtered_issues();
    
    ImGui::Text("Issues (%zu)", filtered_issues.size());
    
    if (filtered_issues.empty()) {
        ImGui::Text("No issues match current filters.");
        return;
    }
    
    // Table with sorting headers
    const ImGuiTableFlags table_flags = ImGuiTableFlags_Resizable | 
                                       ImGuiTableFlags_Sortable |
                                       ImGuiTableFlags_ScrollY |
                                       ImGuiTableFlags_BordersInnerH |
                                       ImGuiTableFlags_RowBg;
    
    if (ImGui::BeginTable("IssuesTable", 5, table_flags)) {
        // Setup columns
        ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Col", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row
        
        // Headers with sorting
        ImGui::TableHeadersRow();
        
        // Handle sorting
        if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs()) {
            if (sort_specs->SpecsDirty) {
                if (sort_specs->SpecsCount > 0) {
                    sort_column_ = sort_specs->Specs[0].ColumnIndex;
                    sort_ascending_ = sort_specs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
                    sort_issues(const_cast<std::vector<const AnalysisIssue*>&>(filtered_issues));
                }
                sort_specs->SpecsDirty = false;
            }
        }
        
        // Render rows
        for (size_t i = 0; i < filtered_issues.size(); ++i) {
            render_issue_row(*filtered_issues[i], static_cast<int>(i));
        }
        
        ImGui::EndTable();
    }
}

void AnalysisResultPanel::render_issue_row(const AnalysisIssue& issue, int index) {
    ImGui::TableNextRow();
    
    // File column (clickable)
    ImGui::TableNextColumn();
    std::string file_name = std::filesystem::path(issue.file).filename().string();
    
    // Add visual indicator for false positives
    if (issue.false_positive) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray out false positives
        file_name = "[FP] " + file_name;
    }
    
    // Create unique ID for this selectable using index to avoid ID collisions
    std::string selectable_id = file_name + "##issue_" + std::to_string(index);
    if (ImGui::Selectable(selectable_id.c_str(), false, ImGuiSelectableFlags_SpanAllColumns)) {
        if (on_file_open_) {
            on_file_open_(issue.file, issue.line, issue.column);
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Click to open: %s:%d:%d", issue.file.c_str(), issue.line, issue.column);
    }
    
    if (issue.false_positive) {
        ImGui::PopStyleColor(); // Restore original text color
    }
    
    // Context menu for false positive management
    std::string popup_id = "IssueContextMenu##" + std::to_string(index);
    if (ImGui::BeginPopupContextItem(popup_id.c_str())) {
        if (issue.false_positive) {
            if (ImGui::MenuItem("Unmark as False Positive")) {
                // Find and update the issue in result_
                for (auto& result_issue : result_.issues) {
                    if (result_issue.file == issue.file && 
                        result_issue.line == issue.line && 
                        result_issue.column == issue.column &&
                        result_issue.id == issue.id) {
                        result_issue.false_positive = false;
                        save_false_positives(); // Auto-save changes
                        break;
                    }
                }
            }
        } else {
            if (ImGui::MenuItem("Mark as False Positive")) {
                // Find and update the issue in result_
                for (auto& result_issue : result_.issues) {
                    if (result_issue.file == issue.file && 
                        result_issue.line == issue.line && 
                        result_issue.column == issue.column &&
                        result_issue.id == issue.id) {
                        result_issue.false_positive = true;
                        save_false_positives(); // Auto-save changes
                        break;
                    }
                }
            }
        }
        ImGui::EndPopup();
    }
    
    // Line column
    ImGui::TableNextColumn();
    if (issue.false_positive) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    }
    ImGui::Text("%d", issue.line);
    if (issue.false_positive) {
        ImGui::PopStyleColor();
    }
    
    // Column column
    ImGui::TableNextColumn();
    if (issue.false_positive) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    }
    ImGui::Text("%d", issue.column);
    if (issue.false_positive) {
        ImGui::PopStyleColor();
    }
    
    // Severity column with colored badge
    ImGui::TableNextColumn();
    if (issue.false_positive) {
        // Draw a grayed-out version of the severity badge for false positives
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        AnalysisIssue temp_issue;
        temp_issue.severity = issue.severity;
        std::string button_id = "[FP] " + temp_issue.severity_string() + "##badge_" + std::to_string(index);
        ImGui::SmallButton(button_id.c_str());
        ImGui::PopStyleColor(3);
    } else {
        draw_severity_badge(issue.severity, index);
    }
    
    // Message column
    ImGui::TableNextColumn();
    if (issue.false_positive) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    }
    ImGui::TextWrapped("[%s] %s", issue.id.c_str(), issue.message.c_str());
    if (issue.false_positive) {
        ImGui::PopStyleColor();
    }
    
    // Show CWE in tooltip if available
    if (ImGui::IsItemHovered() && issue.cwe > 0) {
        ImGui::SetTooltip("CWE-%d: %s", issue.cwe, issue.message.c_str());
    }
}

std::vector<const AnalysisIssue*> AnalysisResultPanel::get_filtered_issues() const {
    std::vector<const AnalysisIssue*> filtered;
    
    for (const auto& issue : result_.issues) {
        // Filter by severity
        bool show_severity = false;
        switch (issue.severity) {
            case IssueSeverity::ERROR: show_severity = show_errors_; break;
            case IssueSeverity::WARNING: show_severity = show_warnings_; break;
            case IssueSeverity::STYLE: show_severity = show_style_; break;
            case IssueSeverity::PERFORMANCE: show_severity = show_performance_; break;
            case IssueSeverity::PORTABILITY: show_severity = show_portability_; break;
            case IssueSeverity::INFORMATION: show_severity = show_information_; break;
        }
        
        if (!show_severity) continue;
        
        // Filter by false positive status
        if (issue.false_positive && !show_false_positives_) continue;
        
        // Filter by text
        if (!issue.matches_filter(filter_text_)) continue;
        
        filtered.push_back(&issue);
    }
    
    // Sort the filtered results
    auto mutable_filtered = filtered;  // Make a mutable copy for sorting
    sort_issues(mutable_filtered);
    
    return mutable_filtered;
}

void AnalysisResultPanel::sort_issues(std::vector<const AnalysisIssue*>& issues) const {
    std::sort(issues.begin(), issues.end(), [this](const AnalysisIssue* a, const AnalysisIssue* b) {
        bool result = false;
        
        switch (sort_column_) {
            case 0: // File
                result = a->file < b->file;
                break;
            case 1: // Line
                result = a->line < b->line;
                break;
            case 2: // Column  
                result = a->column < b->column;
                break;
            case 3: // Severity
                result = static_cast<int>(a->severity) < static_cast<int>(b->severity);
                break;
            case 4: // Message
                result = a->message < b->message;
                break;
        }
        
        return sort_ascending_ ? result : !result;
    });
}

void AnalysisResultPanel::draw_severity_badge(IssueSeverity severity, int index) {
    AnalysisIssue issue;
    issue.severity = severity;
    auto color = issue.severity_color();
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color.r, color.g, color.b, color.a));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, color.a));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.r * 0.6f, color.g * 0.6f, color.b * 0.6f, color.a));
    
    // Create unique ID for the button to avoid ID collisions
    std::string button_id = issue.severity_string() + "##badge_" + std::to_string(index);
    ImGui::SmallButton(button_id.c_str());
    
    ImGui::PopStyleColor(3);
}

void AnalysisResultPanel::set_analysis_result(const AnalysisResult& result) {
    result_ = result;
    
    // Load any existing false positive markings
    load_false_positives();
    
    // Auto-open panel when new results arrive with issues
    if (!result.issues.empty()) {
        set_visible(true);
    }
}

void AnalysisResultPanel::clear_results() {
    result_.clear();
}

void AnalysisResultPanel::save_false_positives(const std::string& project_path) const {
    try {
        std::string fp_file = get_false_positives_file_path(project_path);
        
        nlohmann::json j = nlohmann::json::array();
        
        // Save only false positives with their identifying information
        for (const auto& issue : result_.issues) {
            if (issue.false_positive) {
                nlohmann::json fp_item;
                fp_item["file"] = issue.file;
                fp_item["line"] = issue.line;
                fp_item["column"] = issue.column;
                fp_item["id"] = issue.id;
                fp_item["message"] = issue.message; // For additional verification
                j.push_back(fp_item);
            }
        }
        
        // Write to file
        std::ofstream file(fp_file);
        if (file.is_open()) {
            file << j.dump(2); // Pretty print with 2-space indentation
            file.close();
            std::cout << "[GRAN_AZUL] False positives saved to: " << fp_file << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[GRAN_AZUL] Error saving false positives: " << e.what() << std::endl;
    }
}

void AnalysisResultPanel::load_false_positives(const std::string& project_path) {
    try {
        std::string fp_file = get_false_positives_file_path(project_path);
        
        if (!std::filesystem::exists(fp_file)) {
            return; // No false positives file exists yet
        }
        
        std::ifstream file(fp_file);
        if (!file.is_open()) {
            return;
        }
        
        nlohmann::json j;
        file >> j;
        file.close();
        
        // Mark matching issues as false positives
        if (j.is_array()) {
            for (const auto& fp_item : j) {
                std::string file_path = fp_item["file"];
                int line = fp_item["line"];
                int column = fp_item["column"];
                std::string id = fp_item["id"];
                
                // Find matching issue in current results
                for (auto& issue : result_.issues) {
                    if (issue.file == file_path &&
                        issue.line == line &&
                        issue.column == column &&
                        issue.id == id) {
                        issue.false_positive = true;
                        break;
                    }
                }
            }
        }
        
        std::cout << "[GRAN_AZUL] False positives loaded from: " << fp_file << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[GRAN_AZUL] Error loading false positives: " << e.what() << std::endl;
    }
}

std::string AnalysisResultPanel::get_false_positives_file_path(const std::string& project_path) const {
    if (project_path.empty()) {
        // Use a default location if no project path provided
        return "false_positives.json";
    }
    
    // Create false positives file next to the project file
    std::filesystem::path project_file(project_path);
    std::string fp_filename = project_file.stem().string() + "_false_positives.json";
    return project_file.parent_path() / fp_filename;
}

} // namespace gran_azul::widgets