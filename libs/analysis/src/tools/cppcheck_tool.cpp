#include "tools/cppcheck_tool.h"
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

// ==================== CppcheckConfig Implementation ====================

CppcheckConfig::CppcheckConfig() {
    tool_name = "cppcheck";
    output_file = "cppcheck_analysis.xml";
}

nlohmann::json CppcheckConfig::to_json() const {
    auto j = ToolConfig::to_json();
    
    j["enable_all"] = enable_all;
    j["enable_warning"] = enable_warning;
    j["enable_style"] = enable_style;
    j["enable_performance"] = enable_performance;
    j["enable_portability"] = enable_portability;
    j["enable_information"] = enable_information;
    j["enable_unused_function"] = enable_unused_function;
    j["enable_missing_include"] = enable_missing_include;
    
    j["check_level"] = check_level;
    j["inconclusive"] = inconclusive;
    j["verbose"] = verbose;
    
    j["cpp_standard"] = static_cast<int>(cpp_standard);
    j["platform"] = static_cast<int>(platform);
    
    j["job_count"] = job_count;
    j["quiet"] = quiet;
    
    j["suppress_unused_function"] = suppress_unused_function;
    j["suppress_missing_include_system"] = suppress_missing_include_system;
    j["suppress_missing_include"] = suppress_missing_include;
    j["suppress_duplicate_conditional"] = suppress_duplicate_conditional;
    
    j["use_posix_library"] = use_posix_library;
    j["use_misra_addon"] = use_misra_addon;
    
    return j;
}

void CppcheckConfig::from_json(const nlohmann::json& j) {
    ToolConfig::from_json(j);
    
    enable_all = j.value("enable_all", true);
    enable_warning = j.value("enable_warning", true);
    enable_style = j.value("enable_style", true);
    enable_performance = j.value("enable_performance", true);
    enable_portability = j.value("enable_portability", true);
    enable_information = j.value("enable_information", false);
    enable_unused_function = j.value("enable_unused_function", false);
    enable_missing_include = j.value("enable_missing_include", false);
    
    check_level = j.value("check_level", 0);
    inconclusive = j.value("inconclusive", false);
    verbose = j.value("verbose", false);
    
    cpp_standard = static_cast<CppStandard>(j.value("cpp_standard", static_cast<int>(CppStandard::Cpp20)));
    platform = static_cast<Platform>(j.value("platform", static_cast<int>(Platform::Unix64)));
    
    job_count = j.value("job_count", 4);
    quiet = j.value("quiet", true);
    
    suppress_unused_function = j.value("suppress_unused_function", true);
    suppress_missing_include_system = j.value("suppress_missing_include_system", true);
    suppress_missing_include = j.value("suppress_missing_include", true);
    suppress_duplicate_conditional = j.value("suppress_duplicate_conditional", false);
    
    use_posix_library = j.value("use_posix_library", true);
    use_misra_addon = j.value("use_misra_addon", false);
}

std::unique_ptr<ToolConfig> CppcheckConfig::clone() const {
    auto cloned = std::make_unique<CppcheckConfig>();
    *cloned = *this;
    return cloned;
}

std::string CppcheckConfig::get_display_name() const {
    return "Cppcheck Static Analysis";
}

ValidationResult CppcheckConfig::validate() const {
    ValidationResult result = ToolConfig::validate();
    
    if (job_count < 1 || job_count > 64) {
        result.add_warning("Job count should be between 1 and 64");
    }
    
    if (check_level < 0 || check_level > 1) {
        result.add_error("Check level must be 0 (normal) or 1 (exhaustive)");
    }
    
    return result;
}

std::string CppcheckConfig::get_cpp_standard_string() const {
    switch (cpp_standard) {
        case CppStandard::Cpp03: return "c++03";
        case CppStandard::Cpp11: return "c++11";
        case CppStandard::Cpp14: return "c++14";
        case CppStandard::Cpp17: return "c++17";
        case CppStandard::Cpp20: return "c++20";
        default: return "c++17";
    }
}

std::string CppcheckConfig::get_platform_string() const {
    switch (platform) {
        case Platform::Unix32: return "unix32";
        case Platform::Unix64: return "unix64";
        case Platform::Win32A: return "win32A";
        case Platform::Win64: return "win64";
        default: return "unix64";
    }
}

// ==================== CppcheckTool Implementation ====================

CppcheckTool::CppcheckTool() {
    config_ = std::make_unique<CppcheckConfig>();
}

std::string CppcheckTool::get_name() const {
    return "cppcheck";
}

std::string CppcheckTool::get_version() const {
    if (cached_version_.empty()) {
        cached_version_ = get_cppcheck_version();
    }
    return cached_version_;
}

std::vector<std::string> CppcheckTool::get_supported_extensions() const {
    return {".cpp", ".cxx", ".cc", ".c", ".h", ".hpp", ".hxx"};
}

std::string CppcheckTool::get_description() const {
    return "Cppcheck is a static analysis tool for C/C++ code that detects bugs, undefined behavior, and various code quality issues.";
}

void CppcheckTool::set_configuration(std::unique_ptr<ToolConfig> config) {
    auto cppcheck_config = dynamic_cast<CppcheckConfig*>(config.get());
    if (!cppcheck_config) {
        throw std::invalid_argument("Invalid configuration type for CppcheckTool");
    }
    
    config.release();
    config_.reset(cppcheck_config);
}

const ToolConfig* CppcheckTool::get_configuration() const {
    return config_.get();
}

std::unique_ptr<ToolConfig> CppcheckTool::create_default_config() const {
    return std::make_unique<CppcheckConfig>();
}

ValidationResult CppcheckTool::validate_configuration() const {
    if (!config_) {
        ValidationResult result;
        result.add_error("No configuration set for CppcheckTool");
        return result;
    }
    
    return config_->validate();
}

bool CppcheckTool::is_available() const {
    return !get_executable_path().empty();
}

std::string CppcheckTool::get_executable_path() const {
    if (cached_executable_path_.empty()) {
        cached_executable_path_ = find_cppcheck_executable();
    }
    return cached_executable_path_;
}

std::string CppcheckTool::get_system_requirements() const {
    return "Cppcheck must be installed and available in PATH. Minimum version: 2.0";
}

AnalysisResult CppcheckTool::execute(const AnalysisRequest& request) {
    if (!config_) {
        throw std::runtime_error("No configuration set for CppcheckTool");
    }
    
    if (!is_available()) {
        throw std::runtime_error("Cppcheck is not available on this system");
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
        
        // Execute cppcheck
        wip::utils::process::ProcessExecutor executor;
        wip::utils::process::ProcessResult process_result = executor.execute(
            command_args[0], 
            std::vector<std::string>(command_args.begin() + 1, command_args.end())
        );
        
        auto end_time = std::chrono::steady_clock::now();
        result.execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        if (process_result.exit_code == 0 || process_result.exit_code == 1) {
            // Exit code 1 is normal for cppcheck when issues are found
            result.success = true;
            result = parse_results_file(request.output_file);
            result.execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        } else {
            result.success = false;
            result.error_message = "Cppcheck failed with exit code " + std::to_string(process_result.exit_code) + 
                                 ": " + process_result.stderr_output;
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Cppcheck execution failed: ") + e.what();
    }
    
    analysis_running_ = false;
    return result;
}

std::future<AnalysisResult> CppcheckTool::execute_async(
    const AnalysisRequest& request,
    std::function<void(const AnalysisProgress&)> progress_callback) {
    
    return std::async(std::launch::async, [this, request, progress_callback]() {
        return execute(request);
        // TODO: Implement proper async execution with progress tracking
    });
}

bool CppcheckTool::cancel_analysis() {
    // TODO: Implement analysis cancellation
    return false;
}

bool CppcheckTool::is_analysis_running() const {
    return analysis_running_.load();
}

AnalysisResult CppcheckTool::parse_results_file(const std::string& output_file) {
    if (output_file.size() >= 4 && output_file.substr(output_file.size() - 4) == ".xml") {
        return parse_xml_output(output_file);
    }
    
    throw std::runtime_error("Unsupported cppcheck output format. Only XML is supported.");
}

std::vector<std::string> CppcheckTool::get_supported_output_formats() const {
    return {"xml"};
}

std::vector<std::string> CppcheckTool::build_command_line(const AnalysisRequest& request) const {
    if (!config_) {
        throw std::runtime_error("No configuration set");
    }
    
    std::vector<std::string> args;
    args.push_back(get_executable_path());
    
    // Output format and file
    args.push_back("--xml");
    args.push_back("--xml-version=2");
    args.push_back("--output-file=" + request.output_file);
    
    // Enable checks based on configuration
    if (config_->enable_all) {
        args.push_back("--enable=all");
    } else {
        std::vector<std::string> enabled_checks;
        if (config_->enable_warning) enabled_checks.push_back("warning");
        if (config_->enable_style) enabled_checks.push_back("style");
        if (config_->enable_performance) enabled_checks.push_back("performance");
        if (config_->enable_portability) enabled_checks.push_back("portability");
        if (config_->enable_information) enabled_checks.push_back("information");
        if (config_->enable_unused_function) enabled_checks.push_back("unusedFunction");
        if (config_->enable_missing_include) enabled_checks.push_back("missingInclude");
        
        if (!enabled_checks.empty()) {
            std::string enable_arg = "--enable=";
            for (size_t i = 0; i < enabled_checks.size(); ++i) {
                if (i > 0) enable_arg += ",";
                enable_arg += enabled_checks[i];
            }
            args.push_back(enable_arg);
        }
    }
    
    // Standards and platform
    args.push_back("--std=" + config_->get_cpp_standard_string());
    args.push_back("--platform=" + config_->get_platform_string());
    
    // Performance options
    if (config_->job_count > 1) {
        args.push_back("-j" + std::to_string(config_->job_count));
    }
    
    // Analysis options
    if (config_->inconclusive) {
        args.push_back("--inconclusive");
    }
    
    if (config_->verbose) {
        args.push_back("--verbose");
    }
    
    if (config_->quiet) {
        args.push_back("--quiet");
    }
    
    // Suppressions
    if (config_->suppress_unused_function) {
        args.push_back("--suppress=unusedFunction");
    }
    if (config_->suppress_missing_include_system) {
        args.push_back("--suppress=missingIncludeSystem");
    }
    if (config_->suppress_missing_include) {
        args.push_back("--suppress=missingInclude");
    }
    if (config_->suppress_duplicate_conditional) {
        args.push_back("--suppress=duplicateCondition");
    }
    
    // Libraries
    if (config_->use_posix_library) {
        args.push_back("--library=posix");
    }
    
    // Include paths
    for (const auto& include_path : request.include_paths) {
        args.push_back("-I" + include_path);
    }
    
    // Definitions
    for (const auto& definition : request.definitions) {
        args.push_back("-D" + definition);
    }
    
    // Source path (must be last)
    args.push_back(request.source_path);
    
    return args;
}

std::string CppcheckTool::get_help_text() const {
    return R"(Cppcheck Configuration Options:

Analysis Options:
- Enable All: Enable all available checks
- Enable Warnings: Check for potential bugs and undefined behavior
- Enable Style: Check for coding style issues
- Enable Performance: Check for performance issues
- Enable Portability: Check for portability issues
- Enable Information: Show informational messages
- Inconclusive: Report inconclusive results

Standards:
- C++ Standard: Which C++ standard to use for analysis
- Platform: Target platform (affects type sizes and behavior)

Performance:
- Job Count: Number of parallel analysis threads
- Quiet: Suppress progress messages

Suppressions:
- Various options to suppress common false positives

Libraries:
- POSIX Library: Enable POSIX library checks
- MISRA Addon: Enable MISRA C/C++ compliance checking)";
}

// ==================== Private Helper Methods ====================

std::string CppcheckTool::find_cppcheck_executable() const {
    // Try common locations
    std::vector<std::string> possible_paths = {
        "cppcheck",
        "/usr/bin/cppcheck",
        "/usr/local/bin/cppcheck",
        "/opt/cppcheck/bin/cppcheck"
    };
    
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

std::string CppcheckTool::get_cppcheck_version() const {
    std::string executable = get_executable_path();
    if (executable.empty()) {
        return "Not available";
    }
    
    try {
        wip::utils::process::ProcessExecutor executor;
        auto result = executor.execute(executable, std::vector<std::string>{"--version"});
        if (result.exit_code == 0 && !result.stdout_output.empty()) {
            // Extract version from output like "Cppcheck 2.12.0"
            std::regex version_regex(R"(Cppcheck\s+(\d+\.\d+(?:\.\d+)?))");
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

AnalysisResult CppcheckTool::parse_xml_output(const std::string& xml_file) const {
    AnalysisResult result;
    result.tool_name = get_name();
    result.analysis_id = "parsed-" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    result.timestamp = std::chrono::system_clock::now();
    
    try {
        std::ifstream file(xml_file);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open XML file: " + xml_file);
        }
        
        // Read entire file
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string xml_content = buffer.str();
        
        // Parse XML content (simplified - in production, use a proper XML parser)
        // For now, use regex to extract error elements
        std::regex error_regex(R"(<error\s+[^>]*>)");
        std::sregex_iterator iter(xml_content.begin(), xml_content.end(), error_regex);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            std::string error_tag = iter->str();
            
            AnalysisIssue issue;
            issue.tool_name = get_name();
            
            // Extract attributes using regex (simplified)
            std::regex attr_regex(R"((\w+)=\"([^\"]*)\")");
            std::sregex_iterator attr_iter(error_tag.begin(), error_tag.end(), attr_regex);
            std::sregex_iterator attr_end;
            
            for (; attr_iter != attr_end; ++attr_iter) {
                std::string attr_name = (*attr_iter)[1];
                std::string attr_value = (*attr_iter)[2];
                
                if (attr_name == "id") {
                    issue.rule_id = attr_value;
                    issue.id = attr_value;
                } else if (attr_name == "severity") {
                    issue.severity = map_cppcheck_severity(attr_value);
                    issue.category = map_cppcheck_category(attr_value, issue.rule_id);
                } else if (attr_name == "msg") {
                    issue.message = attr_value;
                } else if (attr_name == "file") {
                    issue.file_path = attr_value;
                } else if (attr_name == "line") {
                    issue.line_number = std::stoi(attr_value);
                } else if (attr_name == "column") {
                    issue.column_number = std::stoi(attr_value);
                }
            }
            
            result.add_issue(issue);
        }
        
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Failed to parse XML output: ") + e.what();
    }
    
    return result;
}

IssueSeverity CppcheckTool::map_cppcheck_severity(const std::string& severity) const {
    if (severity == "error") return IssueSeverity::Error;
    if (severity == "warning") return IssueSeverity::Warning;
    if (severity == "style") return IssueSeverity::Info;
    if (severity == "performance") return IssueSeverity::Warning;
    if (severity == "portability") return IssueSeverity::Warning;
    if (severity == "information") return IssueSeverity::Info;
    return IssueSeverity::Warning;
}

IssueCategory CppcheckTool::map_cppcheck_category(const std::string& severity, const std::string& rule_id) const {
    if (severity == "error") return IssueCategory::Bug;
    if (severity == "warning") return IssueCategory::Bug;
    if (severity == "style") return IssueCategory::Style;
    if (severity == "performance") return IssueCategory::Performance;
    if (severity == "portability") return IssueCategory::Portability;
    if (severity == "information") return IssueCategory::Style;
    
    // Check rule ID for more specific categorization
    if (rule_id.find("security") != std::string::npos ||
        rule_id.find("buffer") != std::string::npos ||
        rule_id.find("null") != std::string::npos) {
        return IssueCategory::Security;
    }
    
    return IssueCategory::Bug;
}

// Static registration
namespace {
    wip::analysis::AnalysisToolRegistration register_cppcheck("cppcheck", 
        []() -> std::unique_ptr<wip::analysis::AnalysisTool> {
            return std::make_unique<wip::analysis::tools::CppcheckTool>();
        });
}

} // namespace tools
} // namespace analysis
} // namespace wip