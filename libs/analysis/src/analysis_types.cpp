#include "analysis_types.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace wip {
namespace analysis {

// ==================== AnalysisIssue Implementation ====================

nlohmann::json AnalysisIssue::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["message"] = message;
    j["file_path"] = file_path;
    j["line_number"] = line_number;
    j["column_number"] = column_number;
    j["severity"] = severity_to_string(severity);
    j["category"] = category_to_string(category);
    j["rule_id"] = rule_id;
    j["tool_name"] = tool_name;
    
    if (fix_suggestion) {
        j["fix_suggestion"] = *fix_suggestion;
    }
    
    return j;
}

AnalysisIssue AnalysisIssue::from_json(const nlohmann::json& j) {
    AnalysisIssue issue;
    issue.id = j.value("id", "");
    issue.message = j.value("message", "");
    issue.file_path = j.value("file_path", "");
    issue.line_number = j.value("line_number", 0);
    issue.column_number = j.value("column_number", 0);
    issue.severity = string_to_severity(j.value("severity", "warning"));
    issue.category = string_to_category(j.value("category", "style"));
    issue.rule_id = j.value("rule_id", "");
    issue.tool_name = j.value("tool_name", "");
    
    if (j.contains("fix_suggestion") && !j["fix_suggestion"].is_null()) {
        issue.fix_suggestion = j["fix_suggestion"].get<std::string>();
    }
    
    return issue;
}

bool AnalysisIssue::operator==(const AnalysisIssue& other) const {
    return id == other.id &&
           message == other.message &&
           file_path == other.file_path &&
           line_number == other.line_number &&
           column_number == other.column_number &&
           severity == other.severity &&
           category == other.category &&
           rule_id == other.rule_id &&
           tool_name == other.tool_name &&
           fix_suggestion == other.fix_suggestion;
}

// ==================== AnalysisResult Implementation ====================

void AnalysisResult::add_issue(const AnalysisIssue& issue) {
    issues.push_back(issue);
    compute_statistics();
}

void AnalysisResult::compute_statistics() {
    issue_counts_by_severity.clear();
    issue_counts_by_category.clear();
    
    for (const auto& issue : issues) {
        issue_counts_by_severity[issue.severity]++;
        issue_counts_by_category[issue.category]++;
    }
}

std::vector<AnalysisIssue> AnalysisResult::get_issues_by_severity(IssueSeverity min_severity) const {
    std::vector<AnalysisIssue> filtered_issues;
    
    for (const auto& issue : issues) {
        if (static_cast<int>(issue.severity) >= static_cast<int>(min_severity)) {
            filtered_issues.push_back(issue);
        }
    }
    
    return filtered_issues;
}

std::vector<AnalysisIssue> AnalysisResult::get_issues_by_category(IssueCategory category) const {
    std::vector<AnalysisIssue> filtered_issues;
    
    for (const auto& issue : issues) {
        if (issue.category == category) {
            filtered_issues.push_back(issue);
        }
    }
    
    return filtered_issues;
}

nlohmann::json AnalysisResult::to_json() const {
    nlohmann::json j;
    j["tool_name"] = tool_name;
    j["analysis_id"] = analysis_id;
    
    // Convert timestamp to ISO 8601 string
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    j["timestamp"] = ss.str();
    
    j["files_analyzed"] = files_analyzed;
    j["execution_time_ms"] = execution_time.count();
    j["success"] = success;
    j["error_message"] = error_message;
    
    // Convert issues to JSON array
    nlohmann::json issues_json = nlohmann::json::array();
    for (const auto& issue : issues) {
        issues_json.push_back(issue.to_json());
    }
    j["issues"] = issues_json;
    
    // Add statistics
    nlohmann::json severity_stats;
    for (const auto& [severity, count] : issue_counts_by_severity) {
        severity_stats[severity_to_string(severity)] = count;
    }
    j["issue_counts_by_severity"] = severity_stats;
    
    nlohmann::json category_stats;
    for (const auto& [category, count] : issue_counts_by_category) {
        category_stats[category_to_string(category)] = count;
    }
    j["issue_counts_by_category"] = category_stats;
    
    return j;
}

AnalysisResult AnalysisResult::from_json(const nlohmann::json& j) {
    AnalysisResult result;
    result.tool_name = j.value("tool_name", "");
    result.analysis_id = j.value("analysis_id", "");
    
    // Parse timestamp from ISO 8601 string
    if (j.contains("timestamp")) {
        std::string timestamp_str = j["timestamp"];
        std::tm tm = {};
        std::stringstream ss(timestamp_str);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        result.timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }
    
    result.files_analyzed = j.value("files_analyzed", 0);
    result.execution_time = std::chrono::milliseconds(j.value("execution_time_ms", 0));
    result.success = j.value("success", false);
    result.error_message = j.value("error_message", "");
    
    // Parse issues
    if (j.contains("issues") && j["issues"].is_array()) {
        for (const auto& issue_json : j["issues"]) {
            result.issues.push_back(AnalysisIssue::from_json(issue_json));
        }
    }
    
    result.compute_statistics();
    return result;
}

bool AnalysisResult::operator==(const AnalysisResult& other) const {
    return tool_name == other.tool_name &&
           analysis_id == other.analysis_id &&
           files_analyzed == other.files_analyzed &&
           execution_time == other.execution_time &&
           success == other.success &&
           error_message == other.error_message &&
           issues == other.issues;
}

// ==================== AnalysisRequest Implementation ====================

nlohmann::json AnalysisRequest::to_json() const {
    nlohmann::json j;
    j["source_path"] = source_path;
    j["output_file"] = output_file;
    j["include_paths"] = include_paths;
    j["definitions"] = definitions;
    j["tool_specific_options"] = tool_specific_options;
    return j;
}

AnalysisRequest AnalysisRequest::from_json(const nlohmann::json& j) {
    AnalysisRequest request;
    request.source_path = j.value("source_path", "");
    request.output_file = j.value("output_file", "");
    
    if (j.contains("include_paths") && j["include_paths"].is_array()) {
        request.include_paths = j["include_paths"].get<std::vector<std::string>>();
    }
    
    if (j.contains("definitions") && j["definitions"].is_array()) {
        request.definitions = j["definitions"].get<std::vector<std::string>>();
    }
    
    if (j.contains("tool_specific_options")) {
        request.tool_specific_options = j["tool_specific_options"];
    }
    
    return request;
}

// ==================== Enum Conversion Functions ====================

std::string severity_to_string(IssueSeverity severity) {
    switch (severity) {
        case IssueSeverity::Info:     return "info";
        case IssueSeverity::Warning:  return "warning";
        case IssueSeverity::Error:    return "error";
        case IssueSeverity::Critical: return "critical";
        default:                      return "unknown";
    }
}

IssueSeverity string_to_severity(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    
    if (lower_str == "info")          return IssueSeverity::Info;
    if (lower_str == "warning")       return IssueSeverity::Warning;
    if (lower_str == "error")         return IssueSeverity::Error;
    if (lower_str == "critical")      return IssueSeverity::Critical;
    
    return IssueSeverity::Warning; // Default fallback
}

std::string category_to_string(IssueCategory category) {
    switch (category) {
        case IssueCategory::Style:           return "style";
        case IssueCategory::Performance:     return "performance";
        case IssueCategory::Security:        return "security";
        case IssueCategory::Bug:             return "bug";
        case IssueCategory::Portability:     return "portability";
        case IssueCategory::Modernization:   return "modernization";
        case IssueCategory::Maintainability: return "maintainability";
        default:                             return "unknown";
    }
}

IssueCategory string_to_category(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    
    if (lower_str == "style")           return IssueCategory::Style;
    if (lower_str == "performance")     return IssueCategory::Performance;
    if (lower_str == "security")        return IssueCategory::Security;
    if (lower_str == "bug")             return IssueCategory::Bug;
    if (lower_str == "portability")     return IssueCategory::Portability;
    if (lower_str == "modernization")   return IssueCategory::Modernization;
    if (lower_str == "maintainability") return IssueCategory::Maintainability;
    
    return IssueCategory::Style; // Default fallback
}

} // namespace analysis
} // namespace wip