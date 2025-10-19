#pragma once

#include "analysis_types.h"
#include <string>
#include <nlohmann/json.hpp>

namespace wip {
namespace analysis {

/**
 * @brief Base configuration class for analysis tools
 * 
 * This class provides common configuration options that all analysis tools
 * share, while allowing derived classes to add tool-specific options.
 */
class ToolConfig {
public:
    virtual ~ToolConfig() = default;
    
    std::string tool_name;                          ///< Name of the analysis tool
    std::string source_path;                        ///< Path to source code to analyze
    std::string output_file;                        ///< Output file for analysis results
    std::vector<std::string> include_paths;         ///< Additional include directories
    std::vector<std::string> definitions;           ///< Preprocessor definitions (-DNAME=value)
    nlohmann::json custom_options;                  ///< Tool-specific options as JSON
    
    /**
     * @brief Convert configuration to JSON representation
     * @return JSON object representing the configuration
     */
    virtual nlohmann::json to_json() const;
    
    /**
     * @brief Load configuration from JSON representation
     * @param j JSON object to parse
     */
    virtual void from_json(const nlohmann::json& j);
    
    /**
     * @brief Create a deep copy of this configuration
     * @return Unique pointer to a copy of this configuration
     */
    virtual std::unique_ptr<ToolConfig> clone() const = 0;
    
    /**
     * @brief Get the display name for this tool configuration
     * @return Human-readable name for UI display
     */
    virtual std::string get_display_name() const {
        return tool_name;
    }
    
    /**
     * @brief Validate the configuration
     * @return Validation result with any errors or warnings
     */
    virtual ValidationResult validate() const;
    
protected:
    /**
     * @brief Validate common configuration options
     * @param result Validation result to add errors/warnings to
     */
    void validate_common_options(ValidationResult& result) const;
};

} // namespace analysis
} // namespace wip