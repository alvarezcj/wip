#pragma once

#include <string>
#include <vector>

namespace gran_azul::widgets {

// Severity levels for analysis issues
enum class IssueSeverity {
    ERROR,
    WARNING, 
    STYLE,
    PERFORMANCE,
    PORTABILITY,
    INFORMATION
};

// Represents a single analysis issue found by cppcheck
struct AnalysisIssue {
    std::string file;           // File path where issue was found
    int line;                   // Line number (1-based)
    int column;                 // Column number (1-based)
    IssueSeverity severity;     // Issue severity level
    std::string id;             // Cppcheck rule ID (e.g., "nullPointer")
    std::string message;        // Human readable description
    int cwe;                    // Common Weakness Enumeration ID (0 if none)
    bool false_positive;        // Whether this issue is marked as false positive
    
    AnalysisIssue() : line(0), column(0), severity(IssueSeverity::INFORMATION), cwe(0), false_positive(false) {}
    
    // Convert severity enum to string for display
    std::string severity_string() const;
    
    // Get severity color for UI display (RGBA values 0.0-1.0)  
    struct Color { float r, g, b, a; };
    Color severity_color() const;
    
    // Check if this issue matches a text filter
    bool matches_filter(const std::string& filter) const;
};

// Represents the complete analysis results
struct AnalysisResult {
    std::vector<AnalysisIssue> issues;
    std::string source_path;        // Path that was analyzed
    std::string timestamp;          // When analysis was performed
    int total_files_analyzed;       // Number of files processed
    bool analysis_successful;       // Whether analysis completed without errors
    std::string error_message;      // Error message if analysis failed
    
    AnalysisResult() : total_files_analyzed(0), analysis_successful(false) {}
    
    // Get issues filtered by severity
    std::vector<const AnalysisIssue*> get_issues_by_severity(IssueSeverity severity) const;
    
    // Get issue count by severity
    size_t count_by_severity(IssueSeverity severity) const;
    
    // Get unique files with issues
    std::vector<std::string> get_affected_files() const;
    
    // Clear all results
    void clear();
};

// Utility functions for parsing cppcheck JSON output
namespace analysis_parser {
    
    // Parse the current cppcheck format
    bool parse_cppcheck_format(const std::string& line, AnalysisIssue& issue);
    
    // Parse a single JSON line from cppcheck output
    bool parse_issue_line(const std::string& json_line, AnalysisIssue& issue);
    
    // Parse complete cppcheck output file and populate result
    bool parse_analysis_file(const std::string& file_path, AnalysisResult& result);
    
    // Convert severity string to enum
    IssueSeverity string_to_severity(const std::string& severity_str);
}

} // namespace gran_azul::widgets