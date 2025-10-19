#pragma once

#include "analysis_tool.h"
#include "tool_config.h"
#include <atomic>
#include <process.h>

namespace wip {
namespace analysis {
namespace tools {

/**
 * @brief Configuration specific to Cppcheck
 */
class CppcheckConfig : public ToolConfig {
public:
    // Analysis options
    bool enable_all = true;
    bool enable_warning = true;
    bool enable_style = true;
    bool enable_performance = true;
    bool enable_portability = true;
    bool enable_information = false;
    bool enable_unused_function = false;
    bool enable_missing_include = false;
    
    // Analysis level
    int check_level = 0; // 0 = normal, 1 = exhaustive
    bool inconclusive = false;
    bool verbose = false;
    
    // Standards and platform
    enum class CppStandard {
        Cpp03 = 0,
        Cpp11 = 1,
        Cpp14 = 2,
        Cpp17 = 3,
        Cpp20 = 4
    };
    
    enum class Platform {
        Unix32 = 0,
        Unix64 = 1,
        Win32A = 2,
        Win64 = 3
    };
    
    CppStandard cpp_standard = CppStandard::Cpp20;
    Platform platform = Platform::Unix64;
    
    // Performance
    int job_count = 4;
    bool quiet = true;
    
    // Suppressions
    bool suppress_unused_function = true;
    bool suppress_missing_include_system = true;
    bool suppress_missing_include = true;
    bool suppress_duplicate_conditional = false;
    
    // Libraries
    bool use_posix_library = true;
    bool use_misra_addon = false;
    
    CppcheckConfig();
    
    // ToolConfig interface
    nlohmann::json to_json() const override;
    void from_json(const nlohmann::json& j) override;
    std::unique_ptr<ToolConfig> clone() const override;
    std::string get_display_name() const override;
    ValidationResult validate() const override;
    
    // Cppcheck-specific methods
    std::string get_cpp_standard_string() const;
    std::string get_platform_string() const;
};

/**
 * @brief Cppcheck static analysis tool implementation
 */
class CppcheckTool : public AnalysisTool {
public:
    CppcheckTool();
    ~CppcheckTool() override = default;
    
    // ==================== Tool Metadata ====================
    std::string get_name() const override;
    std::string get_version() const override;
    std::vector<std::string> get_supported_extensions() const override;
    std::string get_description() const override;
    
    // ==================== Configuration Management ====================
    void set_configuration(std::unique_ptr<ToolConfig> config) override;
    const ToolConfig* get_configuration() const override;
    std::unique_ptr<ToolConfig> create_default_config() const override;
    ValidationResult validate_configuration() const override;
    
    // ==================== Tool Availability ====================
    bool is_available() const override;
    std::string get_executable_path() const override;
    std::string get_system_requirements() const override;
    
    // ==================== Analysis Execution ====================
    AnalysisResult execute(const AnalysisRequest& request) override;
    std::future<AnalysisResult> execute_async(
        const AnalysisRequest& request,
        std::function<void(const AnalysisProgress&)> progress_callback = nullptr) override;
    bool cancel_analysis() override;
    bool is_analysis_running() const override;
    
    // ==================== Result Processing ====================
    AnalysisResult parse_results_file(const std::string& output_file) override;
    std::vector<std::string> get_supported_output_formats() const override;
    
    // ==================== Utility Functions ====================
    std::vector<std::string> build_command_line(const AnalysisRequest& request) const override;
    std::string get_help_text() const override;
    
private:
    std::unique_ptr<CppcheckConfig> config_;
    std::atomic<bool> analysis_running_{false};
    mutable std::string cached_version_;
    mutable std::string cached_executable_path_;
    
    // Helper methods
    std::string find_cppcheck_executable() const;
    std::string get_cppcheck_version() const;
    AnalysisResult parse_xml_output(const std::string& xml_file) const;
    AnalysisIssue parse_xml_error_element(const nlohmann::json& error_json) const;
    IssueSeverity map_cppcheck_severity(const std::string& severity) const;
    IssueCategory map_cppcheck_category(const std::string& severity, const std::string& rule_id) const;
    void validate_cppcheck_config(const CppcheckConfig& config, ValidationResult& result) const;
    
    // Progress tracking
    void track_progress(const std::string& line, 
                       std::function<void(const AnalysisProgress&)> callback,
                       AnalysisProgress& progress) const;
};

} // namespace tools
} // namespace analysis
} // namespace wip