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
    
    // Tool selection
    j["enable_cppcheck"] = data.enable_cppcheck;
    j["enable_clang_tidy"] = data.enable_clang_tidy;
    
    // Cppcheck settings
    nlohmann::json cppcheck_json;
    cppcheck_json["enable_all"] = data.cppcheck.enable_all;
    cppcheck_json["enable_warning"] = data.cppcheck.enable_warning;
    cppcheck_json["enable_style"] = data.cppcheck.enable_style;
    cppcheck_json["enable_performance"] = data.cppcheck.enable_performance;
    cppcheck_json["enable_portability"] = data.cppcheck.enable_portability;
    cppcheck_json["enable_information"] = data.cppcheck.enable_information;
    cppcheck_json["enable_unused_function"] = data.cppcheck.enable_unused_function;
    cppcheck_json["enable_missing_include"] = data.cppcheck.enable_missing_include;
    cppcheck_json["check_level"] = data.cppcheck.check_level;
    cppcheck_json["inconclusive"] = data.cppcheck.inconclusive;
    cppcheck_json["verbose"] = data.cppcheck.verbose;
    cppcheck_json["job_count"] = data.cppcheck.job_count;
    cppcheck_json["quiet"] = data.cppcheck.quiet;
    cppcheck_json["suppress_unused_function"] = data.cppcheck.suppress_unused_function;
    cppcheck_json["suppress_missing_include_system"] = data.cppcheck.suppress_missing_include_system;
    cppcheck_json["suppress_missing_include"] = data.cppcheck.suppress_missing_include;
    cppcheck_json["suppress_duplicate_conditional"] = data.cppcheck.suppress_duplicate_conditional;
    cppcheck_json["use_posix_library"] = data.cppcheck.use_posix_library;
    cppcheck_json["use_misra_addon"] = data.cppcheck.use_misra_addon;
    j["cppcheck"] = cppcheck_json;
    
    // Clang-tidy settings
    nlohmann::json clang_tidy_json;
    clang_tidy_json["enable_bugprone_checks"] = data.clang_tidy.enable_bugprone_checks;
    clang_tidy_json["enable_performance_checks"] = data.clang_tidy.enable_performance_checks;
    clang_tidy_json["enable_modernize_checks"] = data.clang_tidy.enable_modernize_checks;
    clang_tidy_json["enable_readability_checks"] = data.clang_tidy.enable_readability_checks;
    clang_tidy_json["enable_cppcoreguidelines_checks"] = data.clang_tidy.enable_cppcoreguidelines_checks;
    clang_tidy_json["enable_misc_checks"] = data.clang_tidy.enable_misc_checks;
    clang_tidy_json["enable_cert_checks"] = data.clang_tidy.enable_cert_checks;
    clang_tidy_json["disable_magic_numbers"] = data.clang_tidy.disable_magic_numbers;
    clang_tidy_json["disable_uppercase_literal_suffix"] = data.clang_tidy.disable_uppercase_literal_suffix;
    clang_tidy_json["use_color"] = data.clang_tidy.use_color;
    clang_tidy_json["export_fixes"] = data.clang_tidy.export_fixes;
    clang_tidy_json["format_style"] = data.clang_tidy.format_style;
    clang_tidy_json["header_filter_regex_enabled"] = data.clang_tidy.header_filter_regex_enabled;
    clang_tidy_json["header_filter_regex"] = data.clang_tidy.header_filter_regex;
    clang_tidy_json["system_headers"] = data.clang_tidy.system_headers;
    clang_tidy_json["fix_errors"] = data.clang_tidy.fix_errors;
    clang_tidy_json["fix_notes"] = data.clang_tidy.fix_notes;
    clang_tidy_json["config_file"] = data.clang_tidy.config_file;
    j["clang_tidy"] = clang_tidy_json;
    
    // Common settings
    j["cpp_standard"] = data.cpp_standard;
    j["platform"] = data.platform;
    
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
        
        // Tool selection
        if (serialized_data.contains("enable_cppcheck") && serialized_data["enable_cppcheck"].is_boolean()) {
            config.enable_cppcheck = serialized_data["enable_cppcheck"];
        }
        if (serialized_data.contains("enable_clang_tidy") && serialized_data["enable_clang_tidy"].is_boolean()) {
            config.enable_clang_tidy = serialized_data["enable_clang_tidy"];
        }
        
        // Cppcheck settings
        if (serialized_data.contains("cppcheck") && serialized_data["cppcheck"].is_object()) {
            const auto& cppcheck_json = serialized_data["cppcheck"];
            
            if (cppcheck_json.contains("enable_all") && cppcheck_json["enable_all"].is_boolean()) {
                config.cppcheck.enable_all = cppcheck_json["enable_all"];
            }
            if (cppcheck_json.contains("enable_warning") && cppcheck_json["enable_warning"].is_boolean()) {
                config.cppcheck.enable_warning = cppcheck_json["enable_warning"];
            }
            if (cppcheck_json.contains("enable_style") && cppcheck_json["enable_style"].is_boolean()) {
                config.cppcheck.enable_style = cppcheck_json["enable_style"];
            }
            if (cppcheck_json.contains("enable_performance") && cppcheck_json["enable_performance"].is_boolean()) {
                config.cppcheck.enable_performance = cppcheck_json["enable_performance"];
            }
            if (cppcheck_json.contains("enable_portability") && cppcheck_json["enable_portability"].is_boolean()) {
                config.cppcheck.enable_portability = cppcheck_json["enable_portability"];
            }
            if (cppcheck_json.contains("enable_information") && cppcheck_json["enable_information"].is_boolean()) {
                config.cppcheck.enable_information = cppcheck_json["enable_information"];
            }
            if (cppcheck_json.contains("enable_unused_function") && cppcheck_json["enable_unused_function"].is_boolean()) {
                config.cppcheck.enable_unused_function = cppcheck_json["enable_unused_function"];
            }
            if (cppcheck_json.contains("enable_missing_include") && cppcheck_json["enable_missing_include"].is_boolean()) {
                config.cppcheck.enable_missing_include = cppcheck_json["enable_missing_include"];
            }
            if (cppcheck_json.contains("check_level") && cppcheck_json["check_level"].is_number_integer()) {
                config.cppcheck.check_level = cppcheck_json["check_level"];
            }
            if (cppcheck_json.contains("inconclusive") && cppcheck_json["inconclusive"].is_boolean()) {
                config.cppcheck.inconclusive = cppcheck_json["inconclusive"];
            }
            if (cppcheck_json.contains("verbose") && cppcheck_json["verbose"].is_boolean()) {
                config.cppcheck.verbose = cppcheck_json["verbose"];
            }
            if (cppcheck_json.contains("job_count") && cppcheck_json["job_count"].is_number_integer()) {
                config.cppcheck.job_count = cppcheck_json["job_count"];
            }
            if (cppcheck_json.contains("quiet") && cppcheck_json["quiet"].is_boolean()) {
                config.cppcheck.quiet = cppcheck_json["quiet"];
            }
            if (cppcheck_json.contains("suppress_unused_function") && cppcheck_json["suppress_unused_function"].is_boolean()) {
                config.cppcheck.suppress_unused_function = cppcheck_json["suppress_unused_function"];
            }
            if (cppcheck_json.contains("suppress_missing_include_system") && cppcheck_json["suppress_missing_include_system"].is_boolean()) {
                config.cppcheck.suppress_missing_include_system = cppcheck_json["suppress_missing_include_system"];
            }
            if (cppcheck_json.contains("suppress_missing_include") && cppcheck_json["suppress_missing_include"].is_boolean()) {
                config.cppcheck.suppress_missing_include = cppcheck_json["suppress_missing_include"];
            }
            if (cppcheck_json.contains("suppress_duplicate_conditional") && cppcheck_json["suppress_duplicate_conditional"].is_boolean()) {
                config.cppcheck.suppress_duplicate_conditional = cppcheck_json["suppress_duplicate_conditional"];
            }
            if (cppcheck_json.contains("use_posix_library") && cppcheck_json["use_posix_library"].is_boolean()) {
                config.cppcheck.use_posix_library = cppcheck_json["use_posix_library"];
            }
            if (cppcheck_json.contains("use_misra_addon") && cppcheck_json["use_misra_addon"].is_boolean()) {
                config.cppcheck.use_misra_addon = cppcheck_json["use_misra_addon"];
            }
        }
        
        // Clang-tidy settings
        if (serialized_data.contains("clang_tidy") && serialized_data["clang_tidy"].is_object()) {
            const auto& clang_tidy_json = serialized_data["clang_tidy"];
            
            if (clang_tidy_json.contains("enable_bugprone_checks") && clang_tidy_json["enable_bugprone_checks"].is_boolean()) {
                config.clang_tidy.enable_bugprone_checks = clang_tidy_json["enable_bugprone_checks"];
            }
            if (clang_tidy_json.contains("enable_performance_checks") && clang_tidy_json["enable_performance_checks"].is_boolean()) {
                config.clang_tidy.enable_performance_checks = clang_tidy_json["enable_performance_checks"];
            }
            if (clang_tidy_json.contains("enable_modernize_checks") && clang_tidy_json["enable_modernize_checks"].is_boolean()) {
                config.clang_tidy.enable_modernize_checks = clang_tidy_json["enable_modernize_checks"];
            }
            if (clang_tidy_json.contains("enable_readability_checks") && clang_tidy_json["enable_readability_checks"].is_boolean()) {
                config.clang_tidy.enable_readability_checks = clang_tidy_json["enable_readability_checks"];
            }
            if (clang_tidy_json.contains("enable_cppcoreguidelines_checks") && clang_tidy_json["enable_cppcoreguidelines_checks"].is_boolean()) {
                config.clang_tidy.enable_cppcoreguidelines_checks = clang_tidy_json["enable_cppcoreguidelines_checks"];
            }
            if (clang_tidy_json.contains("enable_misc_checks") && clang_tidy_json["enable_misc_checks"].is_boolean()) {
                config.clang_tidy.enable_misc_checks = clang_tidy_json["enable_misc_checks"];
            }
            if (clang_tidy_json.contains("enable_cert_checks") && clang_tidy_json["enable_cert_checks"].is_boolean()) {
                config.clang_tidy.enable_cert_checks = clang_tidy_json["enable_cert_checks"];
            }
            if (clang_tidy_json.contains("disable_magic_numbers") && clang_tidy_json["disable_magic_numbers"].is_boolean()) {
                config.clang_tidy.disable_magic_numbers = clang_tidy_json["disable_magic_numbers"];
            }
            if (clang_tidy_json.contains("disable_uppercase_literal_suffix") && clang_tidy_json["disable_uppercase_literal_suffix"].is_boolean()) {
                config.clang_tidy.disable_uppercase_literal_suffix = clang_tidy_json["disable_uppercase_literal_suffix"];
            }
            if (clang_tidy_json.contains("use_color") && clang_tidy_json["use_color"].is_boolean()) {
                config.clang_tidy.use_color = clang_tidy_json["use_color"];
            }
            if (clang_tidy_json.contains("export_fixes") && clang_tidy_json["export_fixes"].is_boolean()) {
                config.clang_tidy.export_fixes = clang_tidy_json["export_fixes"];
            }
            if (clang_tidy_json.contains("format_style") && clang_tidy_json["format_style"].is_string()) {
                config.clang_tidy.format_style = clang_tidy_json["format_style"];
            }
            if (clang_tidy_json.contains("header_filter_regex_enabled") && clang_tidy_json["header_filter_regex_enabled"].is_boolean()) {
                config.clang_tidy.header_filter_regex_enabled = clang_tidy_json["header_filter_regex_enabled"];
            }
            if (clang_tidy_json.contains("header_filter_regex") && clang_tidy_json["header_filter_regex"].is_string()) {
                config.clang_tidy.header_filter_regex = clang_tidy_json["header_filter_regex"];
            }
            if (clang_tidy_json.contains("system_headers") && clang_tidy_json["system_headers"].is_boolean()) {
                config.clang_tidy.system_headers = clang_tidy_json["system_headers"];
            }
            if (clang_tidy_json.contains("fix_errors") && clang_tidy_json["fix_errors"].is_boolean()) {
                config.clang_tidy.fix_errors = clang_tidy_json["fix_errors"];
            }
            if (clang_tidy_json.contains("fix_notes") && clang_tidy_json["fix_notes"].is_boolean()) {
                config.clang_tidy.fix_notes = clang_tidy_json["fix_notes"];
            }
            if (clang_tidy_json.contains("config_file") && clang_tidy_json["config_file"].is_string()) {
                config.clang_tidy.config_file = clang_tidy_json["config_file"];
            }
        }
        
        // Common settings
        if (serialized_data.contains("cpp_standard") && serialized_data["cpp_standard"].is_number_integer()) {
            config.cpp_standard = serialized_data["cpp_standard"];
        }
        if (serialized_data.contains("platform") && serialized_data["platform"].is_number_integer()) {
            config.platform = serialized_data["platform"];
        }
        
        // Legacy compatibility - map old fields to cppcheck settings if new structure not present
        if (!serialized_data.contains("cppcheck")) {
            if (serialized_data.contains("enable_all") && serialized_data["enable_all"].is_boolean()) {
                config.cppcheck.enable_all = serialized_data["enable_all"];
            }
            if (serialized_data.contains("enable_warning") && serialized_data["enable_warning"].is_boolean()) {
                config.cppcheck.enable_warning = serialized_data["enable_warning"];
            }
            if (serialized_data.contains("enable_style") && serialized_data["enable_style"].is_boolean()) {
                config.cppcheck.enable_style = serialized_data["enable_style"];
            }
            if (serialized_data.contains("job_count") && serialized_data["job_count"].is_number_integer()) {
                config.cppcheck.job_count = serialized_data["job_count"];
            }
            if (serialized_data.contains("quiet") && serialized_data["quiet"].is_boolean()) {
                config.cppcheck.quiet = serialized_data["quiet"];
            }
            // Add other legacy fields as needed...
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