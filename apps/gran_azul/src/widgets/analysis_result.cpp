#include "analysis_result.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <set>
#include <ctime>
#include <iomanip>

namespace gran_azul::widgets {

std::string AnalysisIssue::severity_string() const {
    switch (severity) {
        case IssueSeverity::ERROR: return "Error";
        case IssueSeverity::WARNING: return "Warning";
        case IssueSeverity::STYLE: return "Style";
        case IssueSeverity::PERFORMANCE: return "Performance";
        case IssueSeverity::PORTABILITY: return "Portability";
        case IssueSeverity::INFORMATION: return "Information";
        default: return "Unknown";
    }
}

AnalysisIssue::Color AnalysisIssue::severity_color() const {
    switch (severity) {
        case IssueSeverity::ERROR: return {1.0f, 0.2f, 0.2f, 1.0f};        // Red
        case IssueSeverity::WARNING: return {1.0f, 0.7f, 0.0f, 1.0f};      // Orange
        case IssueSeverity::STYLE: return {0.4f, 0.7f, 1.0f, 1.0f};        // Light Blue
        case IssueSeverity::PERFORMANCE: return {1.0f, 0.4f, 1.0f, 1.0f};  // Magenta
        case IssueSeverity::PORTABILITY: return {0.7f, 0.7f, 0.7f, 1.0f};  // Gray
        case IssueSeverity::INFORMATION: return {0.6f, 0.9f, 0.6f, 1.0f};  // Light Green
        default: return {0.5f, 0.5f, 0.5f, 1.0f};                         // Default Gray
    }
}

bool AnalysisIssue::matches_filter(const std::string& filter) const {
    if (filter.empty()) return true;
    
    std::string lower_filter = filter;
    std::transform(lower_filter.begin(), lower_filter.end(), lower_filter.begin(), ::tolower);
    
    // Check if filter matches any of these fields (case insensitive)
    std::string lower_file = file;
    std::string lower_id = id;
    std::string lower_message = message;
    std::string lower_severity = severity_string();
    
    std::transform(lower_file.begin(), lower_file.end(), lower_file.begin(), ::tolower);
    std::transform(lower_id.begin(), lower_id.end(), lower_id.begin(), ::tolower);
    std::transform(lower_message.begin(), lower_message.end(), lower_message.begin(), ::tolower);
    std::transform(lower_severity.begin(), lower_severity.end(), lower_severity.begin(), ::tolower);
    
    return lower_file.find(lower_filter) != std::string::npos ||
           lower_id.find(lower_filter) != std::string::npos ||
           lower_message.find(lower_filter) != std::string::npos ||
           lower_severity.find(lower_filter) != std::string::npos;
}

std::vector<const AnalysisIssue*> AnalysisResult::get_issues_by_severity(IssueSeverity severity) const {
    std::vector<const AnalysisIssue*> result;
    for (const auto& issue : issues) {
        if (issue.severity == severity) {
            result.push_back(&issue);
        }
    }
    return result;
}

size_t AnalysisResult::count_by_severity(IssueSeverity severity) const {
    size_t count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == severity) {
            count++;
        }
    }
    return count;
}

std::vector<std::string> AnalysisResult::get_affected_files() const {
    std::set<std::string> unique_files;
    for (const auto& issue : issues) {
        unique_files.insert(issue.file);
    }
    return std::vector<std::string>(unique_files.begin(), unique_files.end());
}

void AnalysisResult::clear() {
    issues.clear();
    source_path.clear();
    timestamp.clear();
    total_files_analyzed = 0;
    analysis_successful = false;
    error_message.clear();
}

namespace analysis_parser {

// Simple JSON value extraction - finds value after "key": in JSON line
std::string extract_json_string_value(const std::string& line, const std::string& key) {
    std::string search_key = "\"" + key + "\":";
    size_t pos = line.find(search_key);
    if (pos == std::string::npos) return "";
    
    pos += search_key.length();
    
    // Skip whitespace
    while (pos < line.length() && std::isspace(line[pos])) pos++;
    
    if (pos >= line.length()) return "";
    
    if (line[pos] == '"') {
        // String value
        pos++; // Skip opening quote
        size_t end_pos = pos;
        while (end_pos < line.length() && line[end_pos] != '"') {
            if (line[end_pos] == '\\') end_pos++; // Skip escaped character
            end_pos++;
        }
        if (end_pos < line.length()) {
            return line.substr(pos, end_pos - pos);
        }
    } else {
        // Numeric value
        size_t end_pos = pos;
        while (end_pos < line.length() && (std::isdigit(line[end_pos]) || line[end_pos] == '-')) {
            end_pos++;
        }
        return line.substr(pos, end_pos - pos);
    }
    
    return "";
}

int extract_json_int_value(const std::string& line, const std::string& key) {
    std::string value = extract_json_string_value(line, key);
    if (value.empty()) return 0;
    
    try {
        return std::stoi(value);
    } catch (...) {
        return 0;
    }
}

bool parse_issue_line(const std::string& json_line, AnalysisIssue& issue) {
    // Simple JSON parsing for cppcheck output format
    // Expected format: {"file":"path","line":123,"column":45,"severity":"error","id":"nullPointer","message":"text","cwe":476}
    
    if (json_line.empty() || json_line[0] != '{') {
        return false;
    }
    
    // Extract fields
    issue.file = extract_json_string_value(json_line, "file");
    issue.line = extract_json_int_value(json_line, "line");
    issue.column = extract_json_int_value(json_line, "column");
    issue.id = extract_json_string_value(json_line, "id");
    issue.message = extract_json_string_value(json_line, "message");
    issue.cwe = extract_json_int_value(json_line, "cwe");
    
    std::string severity_str = extract_json_string_value(json_line, "severity");
    issue.severity = string_to_severity(severity_str);
    
    // Validate that we got the essential fields
    return !issue.file.empty() && issue.line > 0 && !issue.id.empty();
}

bool parse_analysis_file(const std::string& file_path, AnalysisResult& result) {
    result.clear();
    
    std::ifstream file(file_path);
    if (!file.is_open()) {
        result.error_message = "Could not open analysis file: " + file_path;
        return false;
    }
    
    std::string line;
    int parsed_issues = 0;
    
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;
        
        AnalysisIssue issue;
        if (parse_issue_line(line, issue)) {
            result.issues.push_back(issue);
            parsed_issues++;
        }
    }
    
    file.close();
    
    // Set result metadata
    result.analysis_successful = true;
    
    // Get current timestamp
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    result.timestamp = oss.str();
    
    // Count unique files
    std::set<std::string> unique_files;
    for (const auto& issue : result.issues) {
        unique_files.insert(issue.file);
    }
    result.total_files_analyzed = static_cast<int>(unique_files.size());
    
    return parsed_issues > 0;
}

IssueSeverity string_to_severity(const std::string& severity_str) {
    std::string lower_str = severity_str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    
    if (lower_str == "error") return IssueSeverity::ERROR;
    if (lower_str == "warning") return IssueSeverity::WARNING;
    if (lower_str == "style") return IssueSeverity::STYLE;
    if (lower_str == "performance") return IssueSeverity::PERFORMANCE;
    if (lower_str == "portability") return IssueSeverity::PORTABILITY;
    if (lower_str == "information") return IssueSeverity::INFORMATION;
    
    return IssueSeverity::INFORMATION; // Default
}

} // namespace analysis_parser

} // namespace gran_azul::widgets