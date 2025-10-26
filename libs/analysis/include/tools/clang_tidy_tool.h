#pragma once

#include "analysis_tool.h"
#include "tool_config.h"
#include <atomic>
#include <process.h>

namespace wip {
namespace analysis {
namespace tools {

/**
 * @brief Configuration specific to Clang-Tidy
 */
class ClangTidyConfig : public ToolConfig {
public:
    // Check configuration
    std::vector<std::string> checks = {
        "bugprone-*", 
        "performance-*", 
        "modernize-*",
        "readability-*",
        "cppcoreguidelines-*"
    };
    
    std::vector<std::string> disabled_checks = {
        "readability-magic-numbers",
        "cppcoreguidelines-avoid-magic-numbers",
        "readability-uppercase-literal-suffix"
    };
    
    // Output options
    bool use_color = false;
    bool export_fixes = false;
    std::string format_style = "file";  // "file", "llvm", "google", etc.
    
    // Configuration file
    std::string config_file;  // Path to .clang-tidy file (optional)
    
    // Analysis options
    bool header_filter_regex_enabled = true;
    std::string header_filter_regex = ".*";
    bool system_headers = false;
    bool fix_errors = false;
    bool fix_notes = false;
    
    ClangTidyConfig();
    
    // ToolConfig interface
    nlohmann::json to_json() const override;
    void from_json(const nlohmann::json& j) override;
    std::unique_ptr<ToolConfig> clone() const override;
    std::string get_display_name() const override;
    ValidationResult validate() const override;
    
    // Clang-tidy specific methods
    std::string get_checks_argument() const;
    bool has_compilation_database() const;
};

/**
 * @brief Clang-Tidy static analysis tool implementation
 */
class ClangTidyTool : public AnalysisTool {
public:
    ClangTidyTool();
    ~ClangTidyTool() override = default;
    
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
        std::function<void(const AnalysisProgress&)> progress_callback = nullptr,
        std::function<void(const std::string&)> output_callback = nullptr) override;
    bool cancel_analysis() override;
    bool is_analysis_running() const override;
    
    // ==================== Result Processing ====================
    AnalysisResult parse_results_file(const std::string& output_file) override;
    std::vector<std::string> get_supported_output_formats() const override;
    
    // ==================== Utility Functions ====================
    std::vector<std::string> build_command_line(const AnalysisRequest& request) const override;
    std::string get_help_text() const override;
    
    // ==================== Clang-Tidy Specific ====================
    
    /**
     * @brief Get list of available checks from clang-tidy
     * @return Vector of available check names
     */
    std::vector<std::string> get_available_checks() const;
    
    /**
     * @brief Check if compile_commands.json exists in build directory
     * @param build_dir Build directory path
     * @return True if compilation database exists
     */
    bool has_compilation_database(const std::string& build_dir) const;
    
private:
    std::unique_ptr<ClangTidyConfig> config_;
    std::atomic<bool> analysis_running_{false};
    mutable std::string cached_version_;
    mutable std::string cached_executable_path_;
    mutable std::vector<std::string> cached_available_checks_;
    
    // Helper methods
    std::string find_clang_tidy_executable() const;
    std::string get_clang_tidy_version() const;
    AnalysisResult parse_clang_tidy_output(const std::string& output) const;
    AnalysisIssue parse_diagnostic_line(const std::string& line) const;
    IssueSeverity map_clang_tidy_severity(const std::string& severity) const;
    IssueCategory map_clang_tidy_category(const std::string& check_name) const;
    void validate_clang_tidy_config(const ClangTidyConfig& config, ValidationResult& result) const;
    
    // Progress tracking for clang-tidy output
    void track_progress_from_output(const std::string& line,
                                   std::function<void(const AnalysisProgress&)> callback,
                                   AnalysisProgress& progress) const;
                                   
    // Find build directory with compile_commands.json
    std::string find_build_directory(const std::string& source_path) const;
};

} // namespace tools
} // namespace analysis
} // namespace wip