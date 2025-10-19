#include "tool_config.h"
#include <filesystem>
#include <algorithm>

namespace wip {
namespace analysis {

nlohmann::json ToolConfig::to_json() const {
    nlohmann::json j;
    j["tool_name"] = tool_name;
    j["source_path"] = source_path;
    j["output_file"] = output_file;
    j["include_paths"] = include_paths;
    j["definitions"] = definitions;
    j["custom_options"] = custom_options;
    return j;
}

void ToolConfig::from_json(const nlohmann::json& j) {
    tool_name = j.value("tool_name", "");
    source_path = j.value("source_path", "");
    output_file = j.value("output_file", "");
    
    if (j.contains("include_paths") && j["include_paths"].is_array()) {
        include_paths = j["include_paths"].get<std::vector<std::string>>();
    }
    
    if (j.contains("definitions") && j["definitions"].is_array()) {
        definitions = j["definitions"].get<std::vector<std::string>>();
    }
    
    if (j.contains("custom_options")) {
        custom_options = j["custom_options"];
    }
}

ValidationResult ToolConfig::validate() const {
    ValidationResult result;
    validate_common_options(result);
    return result;
}

void ToolConfig::validate_common_options(ValidationResult& result) const {
    // Validate tool name
    if (tool_name.empty()) {
        result.add_error("Tool name cannot be empty");
    }
    
    // Validate source path
    if (source_path.empty()) {
        result.add_error("Source path cannot be empty");
    } else {
        std::filesystem::path src_path(source_path);
        if (!std::filesystem::exists(src_path)) {
            result.add_error("Source path does not exist: " + source_path);
        }
    }
    
    // Validate output file path
    if (output_file.empty()) {
        result.add_warning("Output file not specified, results may not be saved");
    } else {
        std::filesystem::path out_path(output_file);
        std::filesystem::path out_dir = out_path.parent_path();
        
        if (!out_dir.empty() && !std::filesystem::exists(out_dir)) {
            result.add_error("Output directory does not exist: " + out_dir.string());
        }
    }
    
    // Validate include paths
    for (const auto& include_path : include_paths) {
        if (!std::filesystem::exists(include_path)) {
            result.add_warning("Include path does not exist: " + include_path);
        }
    }
    
    // Validate definitions format
    for (const auto& definition : definitions) {
        if (definition.empty()) {
            result.add_warning("Empty preprocessor definition found");
            continue;
        }
        
        // Check for basic definition format (NAME or NAME=VALUE)
        if (std::find(definition.begin(), definition.end(), ' ') != definition.end()) {
            result.add_warning("Preprocessor definition contains spaces: " + definition);
        }
    }
}

} // namespace analysis
} // namespace wip