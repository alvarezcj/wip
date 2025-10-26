#pragma once

#include <string>
#include <vector>
#include <json_serializer.h>

namespace gran_azul {

// POD struct representing a Gran Azul project configuration
struct ProjectConfig {
    // Project metadata
    std::string name = "Unnamed Project";
    std::string description = "";
    std::string version = "1.0.0";
    std::string root_path = "./";
    
    // Analysis configuration
    struct AnalysisConfig {
        // Source configuration
        std::string source_path = "./";
        std::string output_file = "analysis_results.json";
        std::string build_dir = "./cppcheck_build";
        
        // Tool selection
        bool enable_cppcheck = true;
        bool enable_clang_tidy = true;
        
        // === Cppcheck Configuration ===
        struct CppcheckSettings {
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
            
            // Custom paths and definitions (previously missing)
            std::vector<std::string> include_paths;
            std::vector<std::string> preprocessor_definitions;
        } cppcheck;
        
        // === Clang-Tidy Configuration ===
        struct ClangTidySettings {
            // Check categories
            bool enable_bugprone_checks = true;
            bool enable_performance_checks = true;
            bool enable_modernize_checks = true;
            bool enable_readability_checks = true;
            bool enable_cppcoreguidelines_checks = true;
            bool enable_misc_checks = true;
            bool enable_cert_checks = false;
            
            // Specific disabled checks
            bool disable_magic_numbers = true;
            bool disable_uppercase_literal_suffix = true;
            
            // Output options
            bool use_color = false;
            bool export_fixes = false;
            std::string format_style = "file";  // "file", "llvm", "google", etc.
            
            // Header filter
            bool header_filter_regex_enabled = true;
            std::string header_filter_regex = ".*";
            bool system_headers = false;
            
            // Fix options
            bool fix_errors = false;
            bool fix_notes = false;
            
            // Configuration file
            std::string config_file;  // Path to .clang-tidy file (optional)
            
            // Custom checks (previously missing)
            std::vector<std::string> additional_checks;
            std::vector<std::string> disabled_checks;
        } clang_tidy;
        
        // === Common Configuration ===
        // Standards and platform
        int cpp_standard = 4; // 0=c++03, 1=c++11, 2=c++14, 3=c++17, 4=c++20
        int platform = 1; // 0=unix32, 1=unix64, 2=win32A, 3=win64
        bool suppress_duplicate_conditional = false;
        
        // Libraries
        bool use_posix_library = true;
        bool use_misra_addon = false;
        
        // Constructor with defaults
        AnalysisConfig() = default;
    } analysis;
    
    // Project files and paths
    std::vector<std::string> source_directories;
    std::vector<std::string> include_directories;
    std::vector<std::string> exclude_patterns;
    
    // Reports and output
    std::string reports_directory = "./reports";
    bool auto_save_results = true;
    bool generate_html_report = false;
    
    // Constructor with defaults
    ProjectConfig() {
        source_directories.push_back("./src");
        include_directories.push_back("./include");
    }
    
    // Utility methods
    std::string get_full_source_path() const;
    std::string get_full_reports_path() const;
    bool is_valid() const;
};

} // namespace gran_azul

// JSON serialization specializations for ProjectConfig types
namespace wip::serialization {

// Specialization for AnalysisConfig
template<>
class Serializer<nlohmann::json, gran_azul::ProjectConfig::AnalysisConfig> {
public:
    using DeserializeResult = std::optional<gran_azul::ProjectConfig::AnalysisConfig>;
    
    nlohmann::json serialize(const gran_azul::ProjectConfig::AnalysisConfig& data) const;
    DeserializeResult deserialize(const nlohmann::json& serialized_data) const;
};

// Specialization for ProjectConfig
template<>
class Serializer<nlohmann::json, gran_azul::ProjectConfig> {
public:
    using DeserializeResult = std::optional<gran_azul::ProjectConfig>;
    
    nlohmann::json serialize(const gran_azul::ProjectConfig& data) const;
    DeserializeResult deserialize(const nlohmann::json& serialized_data) const;
};

} // namespace wip::serialization