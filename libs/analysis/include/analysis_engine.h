#pragma once

#include "analysis_tool.h"
#include "analysis_types.h"
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <future>
#include <atomic>
#include <mutex>

namespace wip {
namespace analysis {

/**
 * @brief Central orchestration engine for multiple analysis tools
 * 
 * The AnalysisEngine manages multiple analysis tools, coordinates their
 * execution, and provides unified result aggregation. It serves as the
 * main business logic layer between the UI and individual tools.
 */
class AnalysisEngine {
public:
    /**
     * @brief Callback for progress updates during analysis
     * @param tool_name Name of the tool providing the update
     * @param progress Current progress information
     */
    using ProgressCallback = std::function<void(const std::string& tool_name, const AnalysisProgress& progress)>;
    
    /**
     * @brief Callback for when analysis completes
     * @param results Vector of results from all tools that ran
     */
    using CompletionCallback = std::function<void(const std::vector<AnalysisResult>& results)>;
    
    AnalysisEngine();
    ~AnalysisEngine();
    
    // ==================== Tool Management ====================
    
    /**
     * @brief Register an analysis tool with the engine
     * @param tool Unique pointer to tool instance
     */
    void register_tool(std::unique_ptr<AnalysisTool> tool);
    
    /**
     * @brief Get list of registered tool names
     * @return Vector of tool names
     */
    std::vector<std::string> get_registered_tools() const;
    
    /**
     * @brief Get list of available (registered and executable) tool names
     * @return Vector of available tool names
     */
    std::vector<std::string> get_available_tools() const;
    
    /**
     * @brief Get tool instance by name
     * @param name Tool name
     * @return Pointer to tool, or nullptr if not found
     */
    AnalysisTool* get_tool(const std::string& name) const;
    
    /**
     * @brief Check if a tool is available
     * @param name Tool name
     * @return True if tool is registered and executable
     */
    bool is_tool_available(const std::string& name) const;
    
    // ==================== Configuration Management ====================
    
    /**
     * @brief Set configuration for a specific tool
     * @param tool_name Name of the tool
     * @param config Configuration to set
     */
    void set_tool_configuration(const std::string& tool_name, std::unique_ptr<ToolConfig> config);
    
    /**
     * @brief Get configuration for a specific tool
     * @param tool_name Name of the tool
     * @return Tool configuration, or nullptr if not found/configured
     */
    const ToolConfig* get_tool_configuration(const std::string& tool_name) const;
    
    /**
     * @brief Validate configuration for a specific tool
     * @param tool_name Name of the tool
     * @return Validation result
     */
    ValidationResult validate_tool_configuration(const std::string& tool_name) const;
    
    /**
     * @brief Validate configurations for all specified tools
     * @param tool_names Names of tools to validate
     * @return Map of tool name to validation result
     */
    std::map<std::string, ValidationResult> validate_configurations(const std::vector<std::string>& tool_names) const;
    
    // ==================== Analysis Execution ====================
    
    /**
     * @brief Execute analysis with a single tool synchronously
     * @param tool_name Name of tool to run
     * @param request Analysis request parameters
     * @return Analysis result
     */
    AnalysisResult analyze_single(const std::string& tool_name, const AnalysisRequest& request);
    
    /**
     * @brief Execute analysis with multiple tools synchronously
     * @param tool_names Names of tools to run
     * @param request Analysis request parameters
     * @return Vector of analysis results (one per tool)
     */
    std::vector<AnalysisResult> analyze_multiple(const std::vector<std::string>& tool_names, 
                                                 const AnalysisRequest& request);
    
    /**
     * @brief Execute analysis with multiple tools asynchronously
     * @param tool_names Names of tools to run
     * @param request Analysis request parameters
     * @param progress_callback Called for progress updates
     * @param completion_callback Called when analysis completes
     * @return Future that will contain the analysis results
     */
    std::future<std::vector<AnalysisResult>> analyze_async(
        const std::vector<std::string>& tool_names,
        const AnalysisRequest& request,
        ProgressCallback progress_callback = nullptr,
        CompletionCallback completion_callback = nullptr);
    
    /**
     * @brief Cancel any running analysis
     * @return True if cancellation was successful
     */
    bool cancel_analysis();
    
    /**
     * @brief Check if analysis is currently running
     * @return True if analysis is in progress
     */
    bool is_analysis_running() const;
    
    // ==================== Result Management ====================
    
    /**
     * @brief Aggregate results from multiple tools into a single result
     * @param results Vector of individual tool results
     * @return Aggregated result containing issues from all tools
     */
    AnalysisResult aggregate_results(const std::vector<AnalysisResult>& results) const;
    
    /**
     * @brief Save analysis results to file
     * @param results Results to save
     * @param file_path Output file path
     */
    void save_results(const std::vector<AnalysisResult>& results, const std::string& file_path) const;
    
    /**
     * @brief Save aggregated results to file
     * @param result Aggregated result to save
     * @param file_path Output file path
     */
    void save_aggregated_result(const AnalysisResult& result, const std::string& file_path) const;
    
    /**
     * @brief Load analysis results from file
     * @param file_path Input file path
     * @return Loaded analysis results
     */
    std::vector<AnalysisResult> load_results(const std::string& file_path) const;
    
    /**
     * @brief Load single aggregated result from file
     * @param file_path Input file path
     * @return Loaded analysis result
     */
    AnalysisResult load_aggregated_result(const std::string& file_path) const;
    
    // ==================== Statistics and Reporting ====================
    
    /**
     * @brief Get analysis statistics across tools
     * @param results Results to analyze
     * @return Statistics summary
     */
    struct AnalysisStatistics {
        size_t total_issues = 0;
        size_t total_files_analyzed = 0;
        std::chrono::milliseconds total_execution_time{0};
        std::map<std::string, size_t> issues_per_tool;
        std::map<IssueSeverity, size_t> issues_by_severity;
        std::map<IssueCategory, size_t> issues_by_category;
        std::vector<std::string> most_problematic_files;  // Files with most issues
    };
    
    AnalysisStatistics get_statistics(const std::vector<AnalysisResult>& results) const;
    
    /**
     * @brief Generate comparison report between different analysis runs
     * @param baseline_results Previous analysis results
     * @param current_results Current analysis results
     * @return Comparison summary
     */
    struct ComparisonReport {
        int issue_count_delta = 0;  // Positive = more issues, negative = fewer issues
        std::vector<AnalysisIssue> new_issues;
        std::vector<AnalysisIssue> resolved_issues;
        std::vector<AnalysisIssue> persistent_issues;
        std::map<IssueSeverity, int> severity_deltas;
    };
    
    ComparisonReport compare_results(const std::vector<AnalysisResult>& baseline_results,
                                    const std::vector<AnalysisResult>& current_results) const;
    
private:
    mutable std::mutex tools_mutex_;
    std::map<std::string, std::unique_ptr<AnalysisTool>> tools_;
    
    std::atomic<bool> analysis_running_{false};
    std::atomic<bool> cancel_requested_{false};
    
    // Helper methods
    void validate_tool_names(const std::vector<std::string>& tool_names) const;
    std::string generate_analysis_id() const;
    
    // Result processing helpers
    void deduplicate_issues(std::vector<AnalysisIssue>& issues) const;
    bool are_issues_similar(const AnalysisIssue& issue1, const AnalysisIssue& issue2) const;
    
    // Statistics helpers
    std::vector<std::string> find_most_problematic_files(const std::vector<AnalysisResult>& results, size_t max_files = 10) const;
};

/**
 * @brief Factory class for creating pre-configured AnalysisEngine instances
 */
class AnalysisEngineFactory {
public:
    /**
     * @brief Create engine with default tools (cppcheck, clang-tidy)
     * @return Configured AnalysisEngine
     */
    static std::unique_ptr<AnalysisEngine> create_default_engine();
    
    /**
     * @brief Create engine with specified tools
     * @param tool_names Names of tools to register
     * @return Configured AnalysisEngine
     */
    static std::unique_ptr<AnalysisEngine> create_engine_with_tools(const std::vector<std::string>& tool_names);
    
    /**
     * @brief Create engine and auto-register all available tools
     * @return Configured AnalysisEngine with all available tools
     */
    static std::unique_ptr<AnalysisEngine> create_full_engine();
};

} // namespace analysis
} // namespace wip