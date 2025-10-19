#include "analysis_engine.h"
#include "tools/cppcheck_tool.h"
#include "tools/clang_tidy_tool.h"
#include <fstream>
#include <algorithm>
#include <future>
#include <set>
#include <filesystem>

namespace wip {
namespace analysis {

// ==================== AnalysisEngine Implementation ====================

AnalysisEngine::AnalysisEngine() = default;

AnalysisEngine::~AnalysisEngine() = default;

void AnalysisEngine::register_tool(std::unique_ptr<AnalysisTool> tool) {
    if (!tool) {
        throw std::invalid_argument("Cannot register null tool");
    }
    
    std::string tool_name = tool->get_name();
    if (tool_name.empty()) {
        throw std::invalid_argument("Tool must have a non-empty name");
    }
    
    std::lock_guard<std::mutex> lock(tools_mutex_);
    tools_[tool_name] = std::move(tool);
}

std::vector<std::string> AnalysisEngine::get_registered_tools() const {
    std::lock_guard<std::mutex> lock(tools_mutex_);
    std::vector<std::string> tool_names;
    tool_names.reserve(tools_.size());
    
    for (const auto& [name, tool] : tools_) {
        tool_names.push_back(name);
    }
    
    return tool_names;
}

std::vector<std::string> AnalysisEngine::get_available_tools() const {
    std::lock_guard<std::mutex> lock(tools_mutex_);
    std::vector<std::string> available_tools;
    
    for (const auto& [name, tool] : tools_) {
        if (tool && tool->is_available()) {
            available_tools.push_back(name);
        }
    }
    
    return available_tools;
}

AnalysisTool* AnalysisEngine::get_tool(const std::string& name) const {
    std::lock_guard<std::mutex> lock(tools_mutex_);
    auto it = tools_.find(name);
    return (it != tools_.end()) ? it->second.get() : nullptr;
}

bool AnalysisEngine::is_tool_available(const std::string& name) const {
    auto tool = get_tool(name);
    return tool && tool->is_available();
}

void AnalysisEngine::set_tool_configuration(const std::string& tool_name, std::unique_ptr<ToolConfig> config) {
    auto tool = get_tool(tool_name);
    if (!tool) {
        throw std::invalid_argument("Tool not found: " + tool_name);
    }
    
    tool->set_configuration(std::move(config));
}

const ToolConfig* AnalysisEngine::get_tool_configuration(const std::string& tool_name) const {
    auto tool = get_tool(tool_name);
    if (!tool) {
        return nullptr;
    }
    
    return tool->get_configuration();
}

ValidationResult AnalysisEngine::validate_tool_configuration(const std::string& tool_name) const {
    auto tool = get_tool(tool_name);
    if (!tool) {
        ValidationResult result;
        result.add_error("Tool not found: " + tool_name);
        return result;
    }
    
    return tool->validate_configuration();
}

std::map<std::string, ValidationResult> AnalysisEngine::validate_configurations(const std::vector<std::string>& tool_names) const {
    std::map<std::string, ValidationResult> results;
    
    for (const auto& tool_name : tool_names) {
        results[tool_name] = validate_tool_configuration(tool_name);
    }
    
    return results;
}

AnalysisResult AnalysisEngine::analyze_single(const std::string& tool_name, const AnalysisRequest& request) {
    validate_tool_names({tool_name});
    
    auto tool = get_tool(tool_name);
    if (!tool) {
        throw std::runtime_error("Tool not found: " + tool_name);
    }
    
    if (!tool->is_available()) {
        throw std::runtime_error("Tool not available: " + tool_name);
    }
    
    analysis_running_ = true;
    cancel_requested_ = false;
    
    try {
        auto result = tool->execute(request);
        analysis_running_ = false;
        return result;
    } catch (...) {
        analysis_running_ = false;
        throw;
    }
}

std::vector<AnalysisResult> AnalysisEngine::analyze_multiple(const std::vector<std::string>& tool_names, 
                                                           const AnalysisRequest& request) {
    validate_tool_names(tool_names);
    
    std::vector<AnalysisResult> results;
    results.reserve(tool_names.size());
    
    analysis_running_ = true;
    cancel_requested_ = false;
    
    try {
        for (const auto& tool_name : tool_names) {
            if (cancel_requested_) {
                break;
            }
            
            auto tool = get_tool(tool_name);
            if (tool && tool->is_available()) {
                // Create a tool-specific request with appropriate output file
                AnalysisRequest tool_request = request;
                if (!tool_request.output_file.empty()) {
                    std::filesystem::path output_path(tool_request.output_file);
                    std::string base_name = output_path.stem().string();
                    std::string extension = output_path.extension().string();
                    tool_request.output_file = base_name + "_" + tool_name + extension;
                }
                
                try {
                    auto result = tool->execute(tool_request);
                    results.push_back(std::move(result));
                } catch (const std::exception& e) {
                    // Create error result for failed tool
                    AnalysisResult error_result;
                    error_result.tool_name = tool_name;
                    error_result.analysis_id = generate_analysis_id();
                    error_result.timestamp = std::chrono::system_clock::now();
                    error_result.success = false;
                    error_result.error_message = std::string("Tool execution failed: ") + e.what();
                    results.push_back(std::move(error_result));
                }
            }
        }
        
        analysis_running_ = false;
        return results;
    } catch (...) {
        analysis_running_ = false;
        throw;
    }
}

std::future<std::vector<AnalysisResult>> AnalysisEngine::analyze_async(
    const std::vector<std::string>& tool_names,
    const AnalysisRequest& request,
    ProgressCallback progress_callback,
    CompletionCallback completion_callback) {
    
    return std::async(std::launch::async, [this, tool_names, request, progress_callback, completion_callback]() {
        try {
            auto results = analyze_multiple(tool_names, request);
            
            if (completion_callback) {
                completion_callback(results);
            }
            
            return results;
        } catch (...) {
            if (completion_callback) {
                completion_callback({});  // Empty results on error
            }
            throw;
        }
    });
}

bool AnalysisEngine::cancel_analysis() {
    cancel_requested_ = true;
    
    // Try to cancel individual tools
    std::lock_guard<std::mutex> lock(tools_mutex_);
    for (const auto& [name, tool] : tools_) {
        if (tool && tool->is_analysis_running()) {
            tool->cancel_analysis();
        }
    }
    
    return true;
}

bool AnalysisEngine::is_analysis_running() const {
    return analysis_running_.load();
}

AnalysisResult AnalysisEngine::aggregate_results(const std::vector<AnalysisResult>& results) const {
    AnalysisResult aggregated;
    aggregated.tool_name = "aggregated";
    aggregated.analysis_id = generate_analysis_id();
    aggregated.timestamp = std::chrono::system_clock::now();
    
    if (results.empty()) {
        aggregated.success = true;
        return aggregated;
    }
    
    // Aggregate all issues
    std::vector<AnalysisIssue> all_issues;
    for (const auto& result : results) {
        for (const auto& issue : result.issues) {
            all_issues.push_back(issue);
        }
        
        // Sum up metadata
        aggregated.files_analyzed += result.files_analyzed;
        aggregated.execution_time += result.execution_time;
        
        // Aggregate success (all must succeed)
        if (!result.success) {
            aggregated.success = false;
            if (!aggregated.error_message.empty()) {
                aggregated.error_message += "; ";
            }
            aggregated.error_message += result.tool_name + ": " + result.error_message;
        }
    }
    
    // Remove duplicates
    deduplicate_issues(all_issues);
    
    // Set aggregated issues
    aggregated.issues = std::move(all_issues);
    aggregated.compute_statistics();
    
    if (aggregated.error_message.empty()) {
        aggregated.success = true;
    }
    
    return aggregated;
}

void AnalysisEngine::save_results(const std::vector<AnalysisResult>& results, const std::string& file_path) const {
    nlohmann::json j = nlohmann::json::array();
    
    for (const auto& result : results) {
        j.push_back(result.to_json());
    }
    
    std::ofstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + file_path);
    }
    
    file << j.dump(2);  // Pretty print with 2-space indentation
}

void AnalysisEngine::save_aggregated_result(const AnalysisResult& result, const std::string& file_path) const {
    auto j = result.to_json();
    
    std::ofstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + file_path);
    }
    
    file << j.dump(2);  // Pretty print with 2-space indentation
}

std::vector<AnalysisResult> AnalysisEngine::load_results(const std::string& file_path) const {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + file_path);
    }
    
    nlohmann::json j;
    file >> j;
    
    std::vector<AnalysisResult> results;
    
    if (j.is_array()) {
        for (const auto& result_json : j) {
            results.push_back(AnalysisResult::from_json(result_json));
        }
    } else {
        // Single result
        results.push_back(AnalysisResult::from_json(j));
    }
    
    return results;
}

AnalysisResult AnalysisEngine::load_aggregated_result(const std::string& file_path) const {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + file_path);
    }
    
    nlohmann::json j;
    file >> j;
    
    return AnalysisResult::from_json(j);
}

AnalysisEngine::AnalysisStatistics AnalysisEngine::get_statistics(const std::vector<AnalysisResult>& results) const {
    AnalysisStatistics stats;
    
    for (const auto& result : results) {
        stats.total_issues += result.issues.size();
        stats.total_files_analyzed += result.files_analyzed;
        stats.total_execution_time += result.execution_time;
        stats.issues_per_tool[result.tool_name] = result.issues.size();
        
        for (const auto& issue : result.issues) {
            stats.issues_by_severity[issue.severity]++;
            stats.issues_by_category[issue.category]++;
        }
    }
    
    stats.most_problematic_files = find_most_problematic_files(results);
    
    return stats;
}

AnalysisEngine::ComparisonReport AnalysisEngine::compare_results(const std::vector<AnalysisResult>& baseline_results,
                                                               const std::vector<AnalysisResult>& current_results) const {
    ComparisonReport report;
    
    // Aggregate results for comparison
    auto baseline_aggregated = aggregate_results(baseline_results);
    auto current_aggregated = aggregate_results(current_results);
    
    report.issue_count_delta = static_cast<int>(current_aggregated.issues.size()) - 
                              static_cast<int>(baseline_aggregated.issues.size());
    
    // Create sets for easier comparison
    std::set<std::string> baseline_issue_ids;
    std::map<std::string, AnalysisIssue> baseline_issues_map;
    
    for (const auto& issue : baseline_aggregated.issues) {
        std::string issue_key = issue.file_path + ":" + std::to_string(issue.line_number) + ":" + issue.rule_id;
        baseline_issue_ids.insert(issue_key);
        baseline_issues_map[issue_key] = issue;
    }
    
    std::set<std::string> current_issue_ids;
    std::map<std::string, AnalysisIssue> current_issues_map;
    
    for (const auto& issue : current_aggregated.issues) {
        std::string issue_key = issue.file_path + ":" + std::to_string(issue.line_number) + ":" + issue.rule_id;
        current_issue_ids.insert(issue_key);
        current_issues_map[issue_key] = issue;
    }
    
    // Find new issues
    for (const auto& issue_key : current_issue_ids) {
        if (baseline_issue_ids.find(issue_key) == baseline_issue_ids.end()) {
            report.new_issues.push_back(current_issues_map[issue_key]);
        }
    }
    
    // Find resolved issues
    for (const auto& issue_key : baseline_issue_ids) {
        if (current_issue_ids.find(issue_key) == current_issue_ids.end()) {
            report.resolved_issues.push_back(baseline_issues_map[issue_key]);
        }
    }
    
    // Find persistent issues
    for (const auto& issue_key : current_issue_ids) {
        if (baseline_issue_ids.find(issue_key) != baseline_issue_ids.end()) {
            report.persistent_issues.push_back(current_issues_map[issue_key]);
        }
    }
    
    // Calculate severity deltas
    for (const auto& [severity, current_count] : current_aggregated.issue_counts_by_severity) {
        size_t baseline_count = 0;
        auto it = baseline_aggregated.issue_counts_by_severity.find(severity);
        if (it != baseline_aggregated.issue_counts_by_severity.end()) {
            baseline_count = it->second;
        }
        report.severity_deltas[severity] = static_cast<int>(current_count) - static_cast<int>(baseline_count);
    }
    
    return report;
}

// ==================== Private Helper Methods ====================

void AnalysisEngine::validate_tool_names(const std::vector<std::string>& tool_names) const {
    if (tool_names.empty()) {
        throw std::invalid_argument("At least one tool name must be specified");
    }
    
    for (const auto& tool_name : tool_names) {
        if (!get_tool(tool_name)) {
            throw std::invalid_argument("Tool not found: " + tool_name);
        }
    }
}

std::string AnalysisEngine::generate_analysis_id() const {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return "analysis_" + std::to_string(timestamp);
}

void AnalysisEngine::deduplicate_issues(std::vector<AnalysisIssue>& issues) const {
    if (issues.size() <= 1) {
        return;
    }
    
    std::vector<AnalysisIssue> unique_issues;
    unique_issues.reserve(issues.size());
    
    for (const auto& issue : issues) {
        bool is_duplicate = false;
        
        for (const auto& unique_issue : unique_issues) {
            if (are_issues_similar(issue, unique_issue)) {
                is_duplicate = true;
                break;
            }
        }
        
        if (!is_duplicate) {
            unique_issues.push_back(issue);
        }
    }
    
    issues = std::move(unique_issues);
}

bool AnalysisEngine::are_issues_similar(const AnalysisIssue& issue1, const AnalysisIssue& issue2) const {
    // Consider issues similar if they have the same file, line, and rule (different tools may report same issue)
    return issue1.file_path == issue2.file_path &&
           issue1.line_number == issue2.line_number &&
           issue1.rule_id == issue2.rule_id;
}

std::vector<std::string> AnalysisEngine::find_most_problematic_files(const std::vector<AnalysisResult>& results, size_t max_files) const {
    std::map<std::string, size_t> file_issue_counts;
    
    for (const auto& result : results) {
        for (const auto& issue : result.issues) {
            file_issue_counts[issue.file_path]++;
        }
    }
    
    // Sort files by issue count
    std::vector<std::pair<std::string, size_t>> sorted_files(file_issue_counts.begin(), file_issue_counts.end());
    std::sort(sorted_files.begin(), sorted_files.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<std::string> result;
    size_t count = std::min(max_files, sorted_files.size());
    result.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        result.push_back(sorted_files[i].first);
    }
    
    return result;
}

// ==================== AnalysisEngineFactory Implementation ====================

std::unique_ptr<AnalysisEngine> AnalysisEngineFactory::create_default_engine() {
    auto engine = std::make_unique<AnalysisEngine>();
    
    // Register default tools
    engine->register_tool(std::make_unique<tools::CppcheckTool>());
    engine->register_tool(std::make_unique<tools::ClangTidyTool>());
    
    return engine;
}

std::unique_ptr<AnalysisEngine> AnalysisEngineFactory::create_engine_with_tools(const std::vector<std::string>& tool_names) {
    auto engine = std::make_unique<AnalysisEngine>();
    
    for (const auto& tool_name : tool_names) {
        if (tool_name == "cppcheck") {
            engine->register_tool(std::make_unique<tools::CppcheckTool>());
        } else if (tool_name == "clang-tidy") {
            engine->register_tool(std::make_unique<tools::ClangTidyTool>());
        }
        // Add more tools as they are implemented
    }
    
    return engine;
}

std::unique_ptr<AnalysisEngine> AnalysisEngineFactory::create_full_engine() {
    return create_default_engine();  // For now, same as default
}

} // namespace analysis
} // namespace wip