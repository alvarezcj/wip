#include "project_config.h"
#include <filesystem>
#include <iostream>

namespace gran_azul {

std::string ProjectConfig::get_full_source_path() const {
    if (std::filesystem::path(analysis.source_path).is_absolute()) {
        return analysis.source_path;
    }
    return std::filesystem::path(root_path) / analysis.source_path;
}

std::string ProjectConfig::get_full_reports_path() const {
    if (std::filesystem::path(reports_directory).is_absolute()) {
        return reports_directory;
    }
    return std::filesystem::path(root_path) / reports_directory;
}

bool ProjectConfig::is_valid() const {
    // Basic validation
    if (name.empty()) return false;
    if (root_path.empty()) return false;
    if (analysis.source_path.empty()) return false;
    
    // Check if paths exist
    if (!std::filesystem::exists(root_path)) return false;
    
    std::string full_source = get_full_source_path();
    if (!std::filesystem::exists(full_source)) return false;
    
    return true;
}

} // namespace gran_azul

// JSON serialization implementations
namespace wip::serialization {

// AnalysisConfig serialization
nlohmann::json Serializer<nlohmann::json, gran_azul::ProjectConfig::AnalysisConfig>::serialize(
    const gran_azul::ProjectConfig::AnalysisConfig& data) const {
    
    nlohmann::json j;
    
    // Source configuration
    j["source_path"] = data.source_path;
    j["output_file"] = data.output_file;
    j["build_dir"] = data.build_dir;
    
    // Analysis options
    j["enable_all"] = data.enable_all;
    j["enable_warning"] = data.enable_warning;
    j["enable_style"] = data.enable_style;
    j["enable_performance"] = data.enable_performance;
    j["enable_portability"] = data.enable_portability;
    j["enable_information"] = data.enable_information;
    j["enable_unused_function"] = data.enable_unused_function;
    j["enable_missing_include"] = data.enable_missing_include;
    
    // Analysis level
    j["check_level"] = data.check_level;
    j["inconclusive"] = data.inconclusive;
    j["verbose"] = data.verbose;
    
    // Standards and platform
    j["cpp_standard"] = data.cpp_standard;
    j["platform"] = data.platform;
    
    // Performance
    j["job_count"] = data.job_count;
    j["quiet"] = data.quiet;
    
    // Suppressions
    j["suppress_unused_function"] = data.suppress_unused_function;
    j["suppress_missing_include_system"] = data.suppress_missing_include_system;
    j["suppress_missing_include"] = data.suppress_missing_include;
    j["suppress_duplicate_conditional"] = data.suppress_duplicate_conditional;
    
    // Libraries
    j["use_posix_library"] = data.use_posix_library;
    j["use_misra_addon"] = data.use_misra_addon;
    
    return j;
}

Serializer<nlohmann::json, gran_azul::ProjectConfig::AnalysisConfig>::DeserializeResult 
Serializer<nlohmann::json, gran_azul::ProjectConfig::AnalysisConfig>::deserialize(
    const nlohmann::json& serialized_data) const {
    
    if (!serialized_data.is_object()) {
        return std::nullopt;
    }
    
    gran_azul::ProjectConfig::AnalysisConfig config;
    
    try {
        // Source configuration
        if (serialized_data.contains("source_path") && serialized_data["source_path"].is_string()) {
            config.source_path = serialized_data["source_path"];
        }
        if (serialized_data.contains("output_file") && serialized_data["output_file"].is_string()) {
            config.output_file = serialized_data["output_file"];
        }
        if (serialized_data.contains("build_dir") && serialized_data["build_dir"].is_string()) {
            config.build_dir = serialized_data["build_dir"];
        }
        
        // Analysis options
        if (serialized_data.contains("enable_all") && serialized_data["enable_all"].is_boolean()) {
            config.enable_all = serialized_data["enable_all"];
        }
        if (serialized_data.contains("enable_warning") && serialized_data["enable_warning"].is_boolean()) {
            config.enable_warning = serialized_data["enable_warning"];
        }
        if (serialized_data.contains("enable_style") && serialized_data["enable_style"].is_boolean()) {
            config.enable_style = serialized_data["enable_style"];
        }
        if (serialized_data.contains("enable_performance") && serialized_data["enable_performance"].is_boolean()) {
            config.enable_performance = serialized_data["enable_performance"];
        }
        if (serialized_data.contains("enable_portability") && serialized_data["enable_portability"].is_boolean()) {
            config.enable_portability = serialized_data["enable_portability"];
        }
        if (serialized_data.contains("enable_information") && serialized_data["enable_information"].is_boolean()) {
            config.enable_information = serialized_data["enable_information"];
        }
        if (serialized_data.contains("enable_unused_function") && serialized_data["enable_unused_function"].is_boolean()) {
            config.enable_unused_function = serialized_data["enable_unused_function"];
        }
        if (serialized_data.contains("enable_missing_include") && serialized_data["enable_missing_include"].is_boolean()) {
            config.enable_missing_include = serialized_data["enable_missing_include"];
        }
        
        // Analysis level
        if (serialized_data.contains("check_level") && serialized_data["check_level"].is_number_integer()) {
            config.check_level = serialized_data["check_level"];
        }
        if (serialized_data.contains("inconclusive") && serialized_data["inconclusive"].is_boolean()) {
            config.inconclusive = serialized_data["inconclusive"];
        }
        if (serialized_data.contains("verbose") && serialized_data["verbose"].is_boolean()) {
            config.verbose = serialized_data["verbose"];
        }
        
        // Standards and platform
        if (serialized_data.contains("cpp_standard") && serialized_data["cpp_standard"].is_number_integer()) {
            config.cpp_standard = serialized_data["cpp_standard"];
        }
        if (serialized_data.contains("platform") && serialized_data["platform"].is_number_integer()) {
            config.platform = serialized_data["platform"];
        }
        
        // Performance
        if (serialized_data.contains("job_count") && serialized_data["job_count"].is_number_integer()) {
            config.job_count = serialized_data["job_count"];
        }
        if (serialized_data.contains("quiet") && serialized_data["quiet"].is_boolean()) {
            config.quiet = serialized_data["quiet"];
        }
        
        // Suppressions
        if (serialized_data.contains("suppress_unused_function") && serialized_data["suppress_unused_function"].is_boolean()) {
            config.suppress_unused_function = serialized_data["suppress_unused_function"];
        }
        if (serialized_data.contains("suppress_missing_include_system") && serialized_data["suppress_missing_include_system"].is_boolean()) {
            config.suppress_missing_include_system = serialized_data["suppress_missing_include_system"];
        }
        if (serialized_data.contains("suppress_missing_include") && serialized_data["suppress_missing_include"].is_boolean()) {
            config.suppress_missing_include = serialized_data["suppress_missing_include"];
        }
        if (serialized_data.contains("suppress_duplicate_conditional") && serialized_data["suppress_duplicate_conditional"].is_boolean()) {
            config.suppress_duplicate_conditional = serialized_data["suppress_duplicate_conditional"];
        }
        
        // Libraries
        if (serialized_data.contains("use_posix_library") && serialized_data["use_posix_library"].is_boolean()) {
            config.use_posix_library = serialized_data["use_posix_library"];
        }
        if (serialized_data.contains("use_misra_addon") && serialized_data["use_misra_addon"].is_boolean()) {
            config.use_misra_addon = serialized_data["use_misra_addon"];
        }
        
        return config;
    } catch (const std::exception& e) {
        std::cerr << "Error deserializing AnalysisConfig: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// ProjectConfig serialization
nlohmann::json Serializer<nlohmann::json, gran_azul::ProjectConfig>::serialize(
    const gran_azul::ProjectConfig& data) const {
    
    nlohmann::json j;
    
    // Project metadata
    j["name"] = data.name;
    j["description"] = data.description;
    j["version"] = data.version;
    j["root_path"] = data.root_path;
    
    // Analysis configuration (nested object)
    JsonSerializer<gran_azul::ProjectConfig::AnalysisConfig> analysis_serializer;
    j["analysis"] = analysis_serializer.serialize(data.analysis);
    
    // Project paths (using vector serialization)
    JsonSerializer<std::vector<std::string>> vector_serializer;
    j["source_directories"] = vector_serializer.serialize(data.source_directories);
    j["include_directories"] = vector_serializer.serialize(data.include_directories);
    j["exclude_patterns"] = vector_serializer.serialize(data.exclude_patterns);
    
    // Reports and output
    j["reports_directory"] = data.reports_directory;
    j["auto_save_results"] = data.auto_save_results;
    j["generate_html_report"] = data.generate_html_report;
    
    return j;
}

Serializer<nlohmann::json, gran_azul::ProjectConfig>::DeserializeResult 
Serializer<nlohmann::json, gran_azul::ProjectConfig>::deserialize(
    const nlohmann::json& serialized_data) const {
    
    if (!serialized_data.is_object()) {
        return std::nullopt;
    }
    
    gran_azul::ProjectConfig project;
    
    try {
        // Project metadata
        if (serialized_data.contains("name") && serialized_data["name"].is_string()) {
            project.name = serialized_data["name"];
        }
        if (serialized_data.contains("description") && serialized_data["description"].is_string()) {
            project.description = serialized_data["description"];
        }
        if (serialized_data.contains("version") && serialized_data["version"].is_string()) {
            project.version = serialized_data["version"];
        }
        if (serialized_data.contains("root_path") && serialized_data["root_path"].is_string()) {
            project.root_path = serialized_data["root_path"];
        }
        
        // Analysis configuration
        if (serialized_data.contains("analysis") && serialized_data["analysis"].is_object()) {
            JsonSerializer<gran_azul::ProjectConfig::AnalysisConfig> analysis_serializer;
            auto analysis_result = analysis_serializer.deserialize(serialized_data["analysis"]);
            if (analysis_result) {
                project.analysis = *analysis_result;
            }
        }
        
        // Project paths
        JsonSerializer<std::vector<std::string>> vector_serializer;
        
        if (serialized_data.contains("source_directories") && serialized_data["source_directories"].is_array()) {
            auto source_dirs = vector_serializer.deserialize(serialized_data["source_directories"]);
            if (source_dirs) {
                project.source_directories = *source_dirs;
            }
        }
        
        if (serialized_data.contains("include_directories") && serialized_data["include_directories"].is_array()) {
            auto include_dirs = vector_serializer.deserialize(serialized_data["include_directories"]);
            if (include_dirs) {
                project.include_directories = *include_dirs;
            }
        }
        
        if (serialized_data.contains("exclude_patterns") && serialized_data["exclude_patterns"].is_array()) {
            auto exclude_pats = vector_serializer.deserialize(serialized_data["exclude_patterns"]);
            if (exclude_pats) {
                project.exclude_patterns = *exclude_pats;
            }
        }
        
        // Reports and output
        if (serialized_data.contains("reports_directory") && serialized_data["reports_directory"].is_string()) {
            project.reports_directory = serialized_data["reports_directory"];
        }
        if (serialized_data.contains("auto_save_results") && serialized_data["auto_save_results"].is_boolean()) {
            project.auto_save_results = serialized_data["auto_save_results"];
        }
        if (serialized_data.contains("generate_html_report") && serialized_data["generate_html_report"].is_boolean()) {
            project.generate_html_report = serialized_data["generate_html_report"];
        }
        
        return project;
    } catch (const std::exception& e) {
        std::cerr << "Error deserializing ProjectConfig: " << e.what() << std::endl;
        return std::nullopt;
    }
}

} // namespace wip::serialization