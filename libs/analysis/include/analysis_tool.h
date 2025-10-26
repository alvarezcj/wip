#pragma once

#include "analysis_types.h"
#include "tool_config.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <future>

namespace wip {
namespace analysis {

/**
 * @brief Abstract base class for all analysis tools
 * 
 * This class defines the interface that all analysis tools must implement.
 * It provides a consistent API for configuring, executing, and retrieving
 * results from different static analysis tools.
 */
class AnalysisTool {
public:
    virtual ~AnalysisTool() = default;
    
    // ==================== Tool Metadata ====================
    
    /**
     * @brief Get the name of this analysis tool
     * @return Tool name (e.g., "cppcheck", "clang-tidy")
     */
    virtual std::string get_name() const = 0;
    
    /**
     * @brief Get the version of this analysis tool
     * @return Version string (e.g., "2.12.0")
     */
    virtual std::string get_version() const = 0;
    
    /**
     * @brief Get file extensions supported by this tool
     * @return Vector of supported extensions (e.g., {".cpp", ".h", ".hpp"})
     */
    virtual std::vector<std::string> get_supported_extensions() const = 0;
    
    /**
     * @brief Get a human-readable description of this tool
     * @return Tool description
     */
    virtual std::string get_description() const = 0;
    
    // ==================== Configuration Management ====================
    
    /**
     * @brief Set the configuration for this tool
     * @param config Configuration to use for analysis
     */
    virtual void set_configuration(std::unique_ptr<ToolConfig> config) = 0;
    
    /**
     * @brief Get the current configuration
     * @return Current tool configuration (may be null if not configured)
     */
    virtual const ToolConfig* get_configuration() const = 0;
    
    /**
     * @brief Create a default configuration for this tool
     * @return Default configuration with sensible defaults
     */
    virtual std::unique_ptr<ToolConfig> create_default_config() const = 0;
    
    /**
     * @brief Validate the current configuration
     * @return Validation result with any errors or warnings
     */
    virtual ValidationResult validate_configuration() const = 0;
    
    // ==================== Tool Availability ====================
    
    /**
     * @brief Check if this tool is available on the system
     * @return True if the tool executable can be found and executed
     */
    virtual bool is_available() const = 0;
    
    /**
     * @brief Get the path to the tool executable
     * @return Path to executable, or empty string if not found
     */
    virtual std::string get_executable_path() const = 0;
    
    /**
     * @brief Get system requirements for this tool
     * @return Human-readable string describing requirements
     */
    virtual std::string get_system_requirements() const = 0;
    
    // ==================== Analysis Execution ====================
    
    /**
     * @brief Execute analysis synchronously
     * @param request Analysis request parameters
     * @return Analysis result
     * @throws std::runtime_error if analysis fails
     */
    virtual AnalysisResult execute(const AnalysisRequest& request) = 0;
    
    /**
     * @brief Execute analysis asynchronously
     * @param request Analysis parameters and configuration
     * @param progress_callback Called periodically with progress updates
     * @param output_callback Called with raw output lines from the tool
     * @return Future that will contain the analysis result
     */
    virtual std::future<AnalysisResult> execute_async(
        const AnalysisRequest& request,
        std::function<void(const AnalysisProgress&)> progress_callback = nullptr,
        std::function<void(const std::string&)> output_callback = nullptr) = 0;
    
    /**
     * @brief Cancel a running analysis (if supported)
     * @return True if cancellation was successful
     */
    virtual bool cancel_analysis() = 0;
    
    /**
     * @brief Check if analysis is currently running
     * @return True if analysis is in progress
     */
    virtual bool is_analysis_running() const = 0;
    
    // ==================== Result Processing ====================
    
    /**
     * @brief Parse analysis results from a file
     * @param output_file Path to the analysis output file
     * @return Parsed analysis result
     * @throws std::runtime_error if parsing fails
     */
    virtual AnalysisResult parse_results_file(const std::string& output_file) = 0;
    
    /**
     * @brief Get supported output formats for this tool
     * @return Vector of format names (e.g., {"xml", "json", "text"})
     */
    virtual std::vector<std::string> get_supported_output_formats() const = 0;
    
    // ==================== Utility Functions ====================
    
    /**
     * @brief Generate command line arguments for this tool
     * @param request Analysis request
     * @return Vector of command line arguments
     */
    virtual std::vector<std::string> build_command_line(const AnalysisRequest& request) const = 0;
    
    /**
     * @brief Get tool-specific help text
     * @return Help text explaining tool options and usage
     */
    virtual std::string get_help_text() const = 0;
};

/**
 * @brief Factory function type for creating analysis tools
 */
using AnalysisToolFactory = std::function<std::unique_ptr<AnalysisTool>()>;

/**
 * @brief Registry for analysis tool factories
 * 
 * This class manages the registration and creation of analysis tools.
 * Tools can register themselves at startup, and the registry can create
 * instances on demand.
 */
class AnalysisToolRegistry {
public:
    /**
     * @brief Get the singleton instance
     */
    static AnalysisToolRegistry& instance();
    
    /**
     * @brief Register a tool factory
     * @param name Tool name
     * @param factory Function to create tool instances
     */
    void register_tool(const std::string& name, AnalysisToolFactory factory);
    
    /**
     * @brief Create a tool instance
     * @param name Tool name
     * @return Unique pointer to tool instance, or nullptr if not found
     */
    std::unique_ptr<AnalysisTool> create_tool(const std::string& name) const;
    
    /**
     * @brief Get list of available tool names
     * @return Vector of registered tool names
     */
    std::vector<std::string> get_available_tools() const;
    
    /**
     * @brief Check if a tool is registered
     * @param name Tool name to check
     * @return True if tool is registered
     */
    bool is_tool_available(const std::string& name) const;
    
    /**
     * @brief Clear all registered tools (primarily for testing)
     */
    void clear();
    
private:
    AnalysisToolRegistry() = default;
    std::map<std::string, AnalysisToolFactory> factories_;
};

/**
 * @brief RAII helper for registering analysis tools
 * 
 * Usage:
 * ```cpp
 * static auto cppcheck_registration = AnalysisToolRegistration(
 *     "cppcheck", []() { return std::make_unique<CppcheckTool>(); }
 * );
 * ```
 */
class AnalysisToolRegistration {
public:
    AnalysisToolRegistration(const std::string& name, AnalysisToolFactory factory) {
        AnalysisToolRegistry::instance().register_tool(name, std::move(factory));
    }
};

} // namespace analysis
} // namespace wip