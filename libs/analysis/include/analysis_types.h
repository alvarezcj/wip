#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <map>
#include <nlohmann/json.hpp>

namespace wip {
namespace analysis {

/**
 * @brief Severity levels for analysis issues
 */
enum class IssueSeverity {
    Info,
    Warning, 
    Error,
    Critical
};

/**
 * @brief Categories of analysis issues
 */
enum class IssueCategory {
    Style,
    Performance,
    Security,
    Bug,
    Portability,
    Modernization,
    Maintainability
};

/**
 * @brief Represents a single issue found during analysis
 */
struct AnalysisIssue {
    std::string id;                                    ///< Unique identifier for this issue
    std::string message;                               ///< Human-readable issue description
    std::string file_path;                             ///< Path to the file containing the issue
    int line_number = 0;                               ///< Line number (1-based, 0 = unknown)
    int column_number = 0;                             ///< Column number (1-based, 0 = unknown)
    IssueSeverity severity = IssueSeverity::Warning;   ///< Issue severity level
    IssueCategory category = IssueCategory::Style;     ///< Issue category
    std::string rule_id;                               ///< Tool-specific rule identifier
    std::string tool_name;                             ///< Name of the tool that found this issue
    std::optional<std::string> fix_suggestion;        ///< Optional automatic fix suggestion
    
    /**
     * @brief Convert issue to JSON representation
     * @return JSON object representing the issue
     */
    nlohmann::json to_json() const;
    
    /**
     * @brief Create issue from JSON representation
     * @param j JSON object to parse
     * @return AnalysisIssue instance
     */
    static AnalysisIssue from_json(const nlohmann::json& j);
    
    /**
     * @brief Equality comparison for testing
     */
    bool operator==(const AnalysisIssue& other) const;
};

/**
 * @brief Analysis execution progress information
 */
struct AnalysisProgress {
    size_t total_files = 0;        ///< Total number of files to analyze
    size_t processed_files = 0;    ///< Number of files processed so far
    std::string current_file;      ///< Currently processing file
    std::string status_message;    ///< Current status description
    
    /**
     * @brief Get progress as percentage (0.0 to 1.0)
     */
    double get_progress_ratio() const {
        if (total_files <= 0) return 0.0;
        double ratio = static_cast<double>(processed_files) / total_files;
        return ratio > 1.0 ? 1.0 : ratio;
    }
};

/**
 * @brief Complete result of an analysis run
 */
struct AnalysisResult {
    std::string tool_name;                                    ///< Name of the analysis tool
    std::string analysis_id;                                  ///< Unique identifier for this analysis
    std::chrono::system_clock::time_point timestamp;         ///< When analysis was performed
    std::vector<AnalysisIssue> issues;                       ///< All issues found
    
    // Execution metadata
    size_t files_analyzed = 0;                                ///< Number of files that were analyzed
    std::chrono::milliseconds execution_time{0};             ///< Total analysis time
    bool success = false;                                     ///< Whether analysis completed successfully
    std::string error_message;                                ///< Error message if analysis failed
    
    // Issue statistics (computed automatically)
    std::map<IssueSeverity, size_t> issue_counts_by_severity; ///< Count of issues by severity
    std::map<IssueCategory, size_t> issue_counts_by_category; ///< Count of issues by category
    
    /**
     * @brief Add an issue to the result and update statistics
     * @param issue Issue to add
     */
    void add_issue(const AnalysisIssue& issue);
    
    /**
     * @brief Compute statistics from current issues
     */
    void compute_statistics();
    
    /**
     * @brief Get total number of issues
     */
    size_t get_total_issue_count() const {
        return issues.size();
    }
    
    /**
     * @brief Get issues filtered by severity
     * @param min_severity Minimum severity level to include
     * @return Vector of issues with severity >= min_severity
     */
    std::vector<AnalysisIssue> get_issues_by_severity(IssueSeverity min_severity) const;
    
    /**
     * @brief Get issues filtered by category
     * @param category Category to filter by
     * @return Vector of issues in the specified category
     */
    std::vector<AnalysisIssue> get_issues_by_category(IssueCategory category) const;
    
    /**
     * @brief Convert result to JSON representation
     * @return JSON object representing the analysis result
     */
    nlohmann::json to_json() const;
    
    /**
     * @brief Create result from JSON representation
     * @param j JSON object to parse
     * @return AnalysisResult instance
     */
    static AnalysisResult from_json(const nlohmann::json& j);
    
    /**
     * @brief Equality comparison for testing
     */
    bool operator==(const AnalysisResult& other) const;
};

/**
 * @brief Request parameters for analysis execution
 */
struct AnalysisRequest {
    std::string source_path;                        ///< Path to analyze (file or directory)
    std::string output_file;                        ///< Where to save analysis output
    std::vector<std::string> include_paths;         ///< Additional include directories
    std::vector<std::string> definitions;           ///< Preprocessor definitions
    nlohmann::json tool_specific_options;           ///< Tool-specific configuration options
    
    /**
     * @brief Convert request to JSON representation
     */
    nlohmann::json to_json() const;
    
    /**
     * @brief Create request from JSON representation
     */
    static AnalysisRequest from_json(const nlohmann::json& j);
};

/**
 * @brief Result of configuration validation
 */
struct ValidationResult {
    bool is_valid = true;                           ///< Whether configuration is valid
    std::vector<std::string> errors;                ///< List of validation errors
    std::vector<std::string> warnings;              ///< List of validation warnings
    
    /**
     * @brief Add validation error
     */
    void add_error(const std::string& error) {
        errors.push_back(error);
        is_valid = false;
    }
    
    /**
     * @brief Add validation warning
     */
    void add_warning(const std::string& warning) {
        warnings.push_back(warning);
    }
    
    /**
     * @brief Check if validation passed without errors or warnings
     */
    bool is_clean() const {
        return is_valid && errors.empty() && warnings.empty();
    }
};

// Utility functions for enum conversions
std::string severity_to_string(IssueSeverity severity);
IssueSeverity string_to_severity(const std::string& str);

std::string category_to_string(IssueCategory category);
IssueCategory string_to_category(const std::string& str);

} // namespace analysis
} // namespace wip