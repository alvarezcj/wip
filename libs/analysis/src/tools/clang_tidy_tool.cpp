#include "tools/clang_tidy_tool.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <algorithm>
#include <future>
#include <process.h>

namespace wip {
namespace analysis {
namespace tools {

// ==================== ClangTidyConfig Implementation ====================

ClangTidyConfig::ClangTidyConfig() {
    tool_name = "clang-tidy";
    output_file = "clang_tidy_analysis.txt";
}

nlohmann::json ClangTidyConfig::to_json() const {
    auto j = ToolConfig::to_json();
    
    j["checks"] = checks;
    j["disabled_checks"] = disabled_checks;
    j["use_color"] = use_color;
    j["export_fixes"] = export_fixes;
    j["format_style"] = format_style;
    j["config_file"] = config_file;
    j["header_filter_regex_enabled"] = header_filter_regex_enabled;
    j["header_filter_regex"] = header_filter_regex;
    j["system_headers"] = system_headers;
    j["fix_errors"] = fix_errors;
    j["fix_notes"] = fix_notes;
    
    return j;
}

void ClangTidyConfig::from_json(const nlohmann::json& j) {
    ToolConfig::from_json(j);
    
    if (j.contains("checks") && j["checks"].is_array()) {
        checks = j["checks"].get<std::vector<std::string>>();
    }
    
    if (j.contains("disabled_checks") && j["disabled_checks"].is_array()) {
        disabled_checks = j["disabled_checks"].get<std::vector<std::string>>();
    }
    
    use_color = j.value("use_color", false);
    export_fixes = j.value("export_fixes", false);
    format_style = j.value("format_style", "file");
    config_file = j.value("config_file", "");
    header_filter_regex_enabled = j.value("header_filter_regex_enabled", true);
    header_filter_regex = j.value("header_filter_regex", ".*");
    system_headers = j.value("system_headers", false);
    fix_errors = j.value("fix_errors", false);
    fix_notes = j.value("fix_notes", false);
}

std::unique_ptr<ToolConfig> ClangTidyConfig::clone() const {
    auto cloned = std::make_unique<ClangTidyConfig>();
    *cloned = *this;
    return cloned;
}

std::string ClangTidyConfig::get_display_name() const {
    return "Clang-Tidy Static Analysis";
}

ValidationResult ClangTidyConfig::validate() const {
    ValidationResult result = ToolConfig::validate();
    
    if (checks.empty()) {
        result.add_warning("No checks enabled - analysis will not find any issues");
    }
    
    if (!config_file.empty() && !std::filesystem::exists(config_file)) {
        result.add_error("Specified config file does not exist: " + config_file);
    }
    
    // Validate header filter regex
    if (header_filter_regex_enabled) {
        try {
            std::regex test_regex(header_filter_regex);
        } catch (const std::regex_error&) {
            result.add_error("Invalid header filter regex: " + header_filter_regex);
        }
    }
    
    return result;
}

std::string ClangTidyConfig::get_checks_argument() const {
    std::string checks_arg;
    
    // Add enabled checks
    for (size_t i = 0; i < checks.size(); ++i) {
        if (i > 0) checks_arg += ",";
        checks_arg += checks[i];
    }
    
    // Add disabled checks with '-' prefix
    for (const auto& disabled_check : disabled_checks) {
        if (!checks_arg.empty()) checks_arg += ",";
        checks_arg += "-" + disabled_check;
    }
    
    return checks_arg;
}

bool ClangTidyConfig::has_compilation_database() const {
    // Check if source path contains build directory with compile_commands.json
    std::filesystem::path source(source_path);
    
    // Common build directory names
    std::vector<std::string> build_dirs = {"build", "Debug", "Release", "_build"};
    
    for (const auto& build_dir : build_dirs) {
        std::filesystem::path build_path = source / build_dir;
        std::filesystem::path compile_commands = build_path / "compile_commands.json";
        
        if (std::filesystem::exists(compile_commands)) {
            return true;
        }
    }
    
    return false;
}

// ==================== ClangTidyTool Implementation ====================

ClangTidyTool::ClangTidyTool() {
    config_ = std::make_unique<ClangTidyConfig>();
}

std::string ClangTidyTool::get_name() const {
    return "clang-tidy";
}

std::string ClangTidyTool::get_version() const {
    if (cached_version_.empty()) {
        cached_version_ = get_clang_tidy_version();
    }
    return cached_version_;
}

std::vector<std::string> ClangTidyTool::get_supported_extensions() const {
    return {".cpp", ".cxx", ".cc", ".c", ".h", ".hpp", ".hxx", ".m", ".mm"};
}

std::string ClangTidyTool::get_description() const {
    return "Clang-Tidy is a clang-based C++ linter tool that provides an extensible framework for diagnosing and fixing typical programming errors, style violations, interface misuse, and bugs.";
}

void ClangTidyTool::set_configuration(std::unique_ptr<ToolConfig> config) {
    auto clang_tidy_config = dynamic_cast<ClangTidyConfig*>(config.get());
    if (!clang_tidy_config) {
        throw std::invalid_argument("Invalid configuration type for ClangTidyTool");
    }
    
    config.release();
    config_.reset(clang_tidy_config);
}

const ToolConfig* ClangTidyTool::get_configuration() const {
    return config_.get();
}

std::unique_ptr<ToolConfig> ClangTidyTool::create_default_config() const {
    return std::make_unique<ClangTidyConfig>();
}

ValidationResult ClangTidyTool::validate_configuration() const {
    if (!config_) {
        ValidationResult result;
        result.add_error("No configuration set for ClangTidyTool");
        return result;
    }
    
    auto result = config_->validate();
    
    // Check for compilation database
    if (!config_->has_compilation_database()) {
        result.add_warning("No compile_commands.json found. Consider running 'cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON' to generate it for better analysis results.");
    }
    
    return result;
}

bool ClangTidyTool::is_available() const {
    return !get_executable_path().empty();
}

std::string ClangTidyTool::get_executable_path() const {
    if (cached_executable_path_.empty()) {
        cached_executable_path_ = find_clang_tidy_executable();
    }
    return cached_executable_path_;
}

std::string ClangTidyTool::get_system_requirements() const {
    return "Clang-Tidy must be installed and available in PATH. Minimum version: 10.0. Requires compile_commands.json for best results.";
}

AnalysisResult ClangTidyTool::execute(const AnalysisRequest& request) {
    if (!config_) {
        throw std::runtime_error("No configuration set for ClangTidyTool");
    }
    
    if (!is_available()) {
        throw std::runtime_error("Clang-Tidy is not available on this system");
    }
    
    analysis_running_ = true;
    
    AnalysisResult result;
    result.tool_name = get_name();
    result.analysis_id = "sync-" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    result.timestamp = std::chrono::system_clock::now();
    
    try {
        auto start_time = std::chrono::steady_clock::now();
        
        // Build command line
        auto command_args = build_command_line(request);
        
        // Execute clang-tidy
        wip::utils::process::ProcessExecutor executor;
        wip::utils::process::ProcessResult process_result = executor.execute(
            command_args[0], 
            std::vector<std::string>(command_args.begin() + 1, command_args.end())
        );
        
        auto end_time = std::chrono::steady_clock::now();
        result.execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // Clang-tidy uses different exit codes:
        // 0 = No issues found
        // 1 = Issues found
        // >1 = Error
        if (process_result.exit_code <= 1) {
            result.success = true;
            
            // Parse output directly (clang-tidy outputs to stdout)
            std::string output_content;
            if (!request.output_file.empty() && std::filesystem::exists(request.output_file)) {
                std::ifstream file(request.output_file);
                std::stringstream buffer;
                buffer << file.rdbuf();
                output_content = buffer.str();
            } else {
                output_content = process_result.stdout_output + "\n" + process_result.stderr_output;
            }
            
            auto parsed_result = parse_clang_tidy_output(output_content);
            result.issues = std::move(parsed_result.issues);
            result.compute_statistics();
            
        } else {
            result.success = false;
            result.error_message = "Clang-Tidy failed with exit code " + std::to_string(process_result.exit_code) + 
                                 ": " + process_result.stderr_output;
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Clang-Tidy execution failed: ") + e.what();
    }
    
    analysis_running_ = false;
    return result;
}

std::future<AnalysisResult> ClangTidyTool::execute_async(
    const AnalysisRequest& request,
    std::function<void(const AnalysisProgress&)> progress_callback) {
    
    return std::async(std::launch::async, [this, request, progress_callback]() {
        return execute(request);
        // TODO: Implement proper async execution with progress tracking
    });
}

bool ClangTidyTool::cancel_analysis() {
    // TODO: Implement analysis cancellation
    return false;
}

bool ClangTidyTool::is_analysis_running() const {
    return analysis_running_.load();
}

AnalysisResult ClangTidyTool::parse_results_file(const std::string& output_file) {
    std::ifstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open output file: " + output_file);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    return parse_clang_tidy_output(content);
}

std::vector<std::string> ClangTidyTool::get_supported_output_formats() const {
    return {"text", "yaml"};
}

std::vector<std::string> ClangTidyTool::build_command_line(const AnalysisRequest& request) const {
    if (!config_) {
        throw std::runtime_error("No configuration set");
    }
    
    std::vector<std::string> args;
    args.push_back(get_executable_path());
    
    // Checks configuration
    std::string checks_arg = config_->get_checks_argument();
    if (!checks_arg.empty()) {
        args.push_back("--checks=" + checks_arg);
    }
    
    // Build database path
    std::string build_dir = find_build_directory(request.source_path);
    if (!build_dir.empty()) {
        args.push_back("-p");
        args.push_back(build_dir);
    }
    
    // Configuration file
    if (!config_->config_file.empty()) {
        args.push_back("--config-file=" + config_->config_file);
    }
    
    // Header filter
    if (config_->header_filter_regex_enabled) {
        args.push_back("--header-filter=" + config_->header_filter_regex);
    }
    
    // Output options
    if (config_->use_color) {
        args.push_back("--use-color");
    }
    
    if (config_->export_fixes) {
        std::string fixes_file = request.output_file;
        if (fixes_file.empty()) {
            fixes_file = "clang_tidy_fixes.yaml";
        } else {
            size_t dot_pos = fixes_file.find_last_of('.');
            if (dot_pos != std::string::npos) {
                fixes_file = fixes_file.substr(0, dot_pos) + "_fixes.yaml";
            } else {
                fixes_file += "_fixes.yaml";
            }
        }
        args.push_back("--export-fixes=" + fixes_file);
    }
    
    // Format style
    if (!config_->format_style.empty() && config_->format_style != "file") {
        args.push_back("--format-style=" + config_->format_style);
    }
    
    // System headers
    if (config_->system_headers) {
        args.push_back("--system-headers");
    }
    
    // Additional options for include paths and definitions
    for (const auto& include_path : request.include_paths) {
        args.push_back("--extra-arg=-I" + include_path);
    }
    
    for (const auto& definition : request.definitions) {
        args.push_back("--extra-arg=-D" + definition);
    }
    
    // Source file (clang-tidy analyzes single files)
    if (std::filesystem::is_regular_file(request.source_path)) {
        args.push_back(request.source_path);
    } else if (std::filesystem::is_directory(request.source_path)) {
        // For directories, we need to find C++ files
        // This is simplified - in practice, you'd want to handle this better
        for (const auto& entry : std::filesystem::recursive_directory_iterator(request.source_path)) {
            if (entry.is_regular_file()) {
                auto ext = entry.path().extension().string();
                if (std::find(get_supported_extensions().begin(), get_supported_extensions().end(), ext) 
                    != get_supported_extensions().end()) {
                    args.push_back(entry.path().string());
                    break; // For now, just analyze the first file found
                }
            }
        }
    }
    
    return args;
}

std::string ClangTidyTool::get_help_text() const {
    return R"(Clang-Tidy Configuration Options:

Check Configuration:
- Enabled Checks: List of check patterns to enable (e.g., "bugprone-*", "performance-*")
- Disabled Checks: List of specific checks to disable

Analysis Options:
- Header Filter: Regex pattern for headers to analyze
- System Headers: Whether to analyze system headers
- Configuration File: Path to .clang-tidy configuration file

Output Options:
- Use Color: Enable colored output
- Export Fixes: Export automatic fixes to YAML file
- Format Style: Code formatting style for fixes

Requirements:
- Clang-Tidy requires compile_commands.json for best results
- Generate with: cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
- Supports C, C++, Objective-C files)";
}

std::vector<std::string> ClangTidyTool::get_available_checks() const {
    if (!cached_available_checks_.empty()) {
        return cached_available_checks_;
    }
    
    std::string executable = get_executable_path();
    if (executable.empty()) {
        return {};
    }
    
    try {
        wip::utils::process::ProcessExecutor executor;
        auto result = executor.execute(executable, std::vector<std::string>{"--list-checks", "--checks=*"});
        if (result.exit_code == 0) {
            std::istringstream stream(result.stdout_output);
            std::string line;
            std::vector<std::string> checks;
            
            while (std::getline(stream, line)) {
                // Skip header lines and empty lines
                if (line.find("Enabled checks:") != std::string::npos ||
                    line.find("clang-analyzer-") == 0 ||
                    line.empty()) {
                    continue;
                }
                
                // Trim whitespace
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);
                
                if (!line.empty()) {
                    checks.push_back(line);
                }
            }
            
            cached_available_checks_ = checks;
            return checks;
        }
    } catch (...) {
        // Ignore errors
    }
    
    return {};
}

bool ClangTidyTool::has_compilation_database(const std::string& build_dir) const {
    std::filesystem::path compile_commands = std::filesystem::path(build_dir) / "compile_commands.json";
    return std::filesystem::exists(compile_commands);
}

// ==================== Private Helper Methods ====================

std::string ClangTidyTool::find_clang_tidy_executable() const {
    // Try common locations
    std::vector<std::string> possible_paths = {
        "clang-tidy",
        "/usr/bin/clang-tidy",
        "/usr/local/bin/clang-tidy",
        "/opt/llvm/bin/clang-tidy"
    };
    
    // Also try versioned executables
    for (int version = 18; version >= 10; --version) {
        possible_paths.push_back("clang-tidy-" + std::to_string(version));
        possible_paths.push_back("/usr/bin/clang-tidy-" + std::to_string(version));
    }
    
    for (const auto& path : possible_paths) {
        try {
            wip::utils::process::ProcessExecutor executor;
            auto result = executor.execute(path, std::vector<std::string>{"--version"});
            if (result.exit_code == 0) {
                return path;
            }
        } catch (...) {
            continue;
        }
    }
    
    return "";
}

std::string ClangTidyTool::get_clang_tidy_version() const {
    std::string executable = get_executable_path();
    if (executable.empty()) {
        return "Not available";
    }
    
    try {
        wip::utils::process::ProcessExecutor executor;
        auto result = executor.execute(executable, std::vector<std::string>{"--version"});
        if (result.exit_code == 0 && !result.stdout_output.empty()) {
            // Extract version from output like "LLVM (http://llvm.org/):\n  LLVM version 14.0.0"
            std::regex version_regex(R"(LLVM version\s+(\d+\.\d+(?:\.\d+)?))");
            std::smatch matches;
            if (std::regex_search(result.stdout_output, matches, version_regex)) {
                return matches[1].str();
            }
        }
    } catch (...) {
        // Ignore errors
    }
    
    return "Unknown";
}

AnalysisResult ClangTidyTool::parse_clang_tidy_output(const std::string& output) const {
    AnalysisResult result;
    result.tool_name = get_name();
    result.analysis_id = "parsed-" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    result.timestamp = std::chrono::system_clock::now();
    
    try {
        std::istringstream stream(output);
        std::string line;
        
        while (std::getline(stream, line)) {
            auto issue = parse_diagnostic_line(line);
            if (!issue.file_path.empty()) {
                result.add_issue(issue);
            }
        }
        
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Failed to parse clang-tidy output: ") + e.what();
    }
    
    return result;
}

AnalysisIssue ClangTidyTool::parse_diagnostic_line(const std::string& line) const {
    AnalysisIssue issue;
    
    // Clang-tidy output format: /path/to/file.cpp:123:45: warning: message [check-name]
    std::regex diagnostic_regex(R"(^([^:]+):(\d+):(\d+):\s+(warning|error|note):\s+([^[]+)\s*(?:\[([^\]]+)\])?.*$)");
    std::smatch matches;
    
    if (std::regex_match(line, matches, diagnostic_regex)) {
        issue.file_path = matches[1].str();
        issue.line_number = std::stoi(matches[2].str());
        issue.column_number = std::stoi(matches[3].str());
        
        std::string severity_str = matches[4].str();
        issue.severity = map_clang_tidy_severity(severity_str);
        
        issue.message = matches[5].str();
        // Trim trailing whitespace
        issue.message.erase(issue.message.find_last_not_of(" \t") + 1);
        
        if (matches[6].matched) {
            issue.rule_id = matches[6].str();
            issue.category = map_clang_tidy_category(issue.rule_id);
        }
        
        issue.tool_name = get_name();
        issue.id = issue.rule_id + "_" + std::to_string(issue.line_number) + "_" + std::to_string(issue.column_number);
    }
    
    return issue;
}

IssueSeverity ClangTidyTool::map_clang_tidy_severity(const std::string& severity) const {
    if (severity == "error") return IssueSeverity::Error;
    if (severity == "warning") return IssueSeverity::Warning;
    if (severity == "note") return IssueSeverity::Info;
    return IssueSeverity::Warning;
}

IssueCategory ClangTidyTool::map_clang_tidy_category(const std::string& check_name) const {
    if (check_name.find("performance-") == 0) return IssueCategory::Performance;
    if (check_name.find("bugprone-") == 0) return IssueCategory::Bug;
    if (check_name.find("security") != std::string::npos || 
        check_name.find("cert-") == 0) return IssueCategory::Security;
    if (check_name.find("modernize-") == 0) return IssueCategory::Modernization;
    if (check_name.find("readability-") == 0 || 
        check_name.find("style") != std::string::npos) return IssueCategory::Style;
    if (check_name.find("portability") != std::string::npos) return IssueCategory::Portability;
    if (check_name.find("maintainability") != std::string::npos) return IssueCategory::Maintainability;
    
    // Default categorization based on severity implications
    return IssueCategory::Bug;
}

std::string ClangTidyTool::find_build_directory(const std::string& source_path) const {
    std::filesystem::path source(source_path);
    
    // If source_path is a file, get its directory
    if (std::filesystem::is_regular_file(source)) {
        source = source.parent_path();
    }
    
    // Common build directory names to search for
    std::vector<std::string> build_dirs = {"build", "_build", "Debug", "Release"};
    
    // Search up the directory tree
    std::filesystem::path current = source;
    while (!current.empty() && current != current.parent_path()) {
        for (const auto& build_dir : build_dirs) {
            std::filesystem::path build_path = current / build_dir;
            if (has_compilation_database(build_path.string())) {
                return build_path.string();
            }
        }
        current = current.parent_path();
    }
    
    return "";
}

// Static registration
namespace {
    wip::analysis::AnalysisToolRegistration register_clang_tidy("clang-tidy", 
        []() -> std::unique_ptr<wip::analysis::AnalysisTool> {
            return std::make_unique<wip::analysis::tools::ClangTidyTool>();
        });
}

} // namespace tools
} // namespace analysis
} // namespace wip