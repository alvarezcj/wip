#pragma once

#include <widgets.h>
#include "analysis_result.h"
#include <functional>

namespace gran_azul::widgets {

// Callback type for when user wants to open a file at specific location
using FileOpenCallback = std::function<void(const std::string& file_path, int line, int column)>;

class AnalysisResultPanel : public wip::gui::Panel {
private:
    AnalysisResult result_;
    char filter_text_[256] = "";
    bool show_errors_ = true;
    bool show_warnings_ = true;
    bool show_style_ = true;
    bool show_performance_ = true;
    bool show_portability_ = true;
    bool show_information_ = true;
    int sort_column_ = 0; // 0=file, 1=line, 2=severity, 3=message
    bool sort_ascending_ = true;
    
    FileOpenCallback on_file_open_;
    
public:
    AnalysisResultPanel(const std::string& title = "Analysis Results");
    
    // Panel interface
    void update(float delta_time) override;
    void draw_content() override;
    
    // Result management
    void set_analysis_result(const AnalysisResult& result);
    const AnalysisResult& get_analysis_result() const { return result_; }
    void clear_results();
    
    // Callback setters
    void set_file_open_callback(FileOpenCallback callback) { on_file_open_ = callback; }
    
private:
    void render_summary();
    void render_filters();
    void render_issues_table();
    void render_issue_row(const AnalysisIssue& issue, int index);
    
    // Get filtered and sorted issues for display
    std::vector<const AnalysisIssue*> get_filtered_issues() const;
    
    // Sorting helper
    void sort_issues(std::vector<const AnalysisIssue*>& issues) const;
    
    // UI helpers
    void draw_severity_badge(IssueSeverity severity, int index);
    const char* get_sort_arrow(int column) const;
};

} // namespace gran_azul::widgets