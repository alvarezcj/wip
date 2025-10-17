#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <json_serializer.h>

namespace gran_azul {

struct AnalysisTool {
    std::string name;
    std::string version;
    std::string description;
    std::chrono::system_clock::time_point execution_time;
    std::chrono::milliseconds duration;
    bool success;
    std::string error_message;
    
    // Tool-specific data (JSON object)
    nlohmann::json results;
};

struct ProjectSummary {
    std::string name;
    std::string root_path;
    std::string project_file;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_modified;
    std::vector<std::string> source_paths;
    std::vector<std::string> build_paths;
    
    // Statistics
    size_t total_files;
    size_t lines_of_code;
    std::vector<std::string> file_extensions;
};

struct ComprehensiveReport {
    // Report metadata
    std::string report_version = "1.0";
    std::chrono::system_clock::time_point generated_at;
    std::string generator_name = "Gran Azul";
    std::string generator_version = "1.0.0";
    
    // Project information
    ProjectSummary project;
    
    // Analysis tools and their results
    std::vector<AnalysisTool> tools;
    
    // Aggregated statistics
    struct Statistics {
        size_t total_issues = 0;
        size_t critical_issues = 0;
        size_t major_issues = 0;
        size_t minor_issues = 0;
        size_t info_issues = 0;
        
        // Coverage information (for future tools)
        double code_coverage_percentage = 0.0;
        size_t covered_lines = 0;
        size_t total_lines = 0;
        
        // Quality metrics
        double quality_score = 0.0; // 0-100
        std::string quality_rating = "Unknown"; // A, B, C, D, E
    } statistics;
    
    // Summary and recommendations
    std::vector<std::string> recommendations;
    std::vector<std::string> warnings;
    std::string overall_assessment;
};

class ReportGenerator {
private:
    std::string project_base_path_;
    
public:
    ReportGenerator(const std::string& project_base_path);
    
    // Generate comprehensive report
    ComprehensiveReport generate_report(const ProjectSummary& project_info);
    
    // Add analysis tool results
    void add_cppcheck_results(ComprehensiveReport& report, const std::string& cppcheck_output_file);
    void add_tool_result(ComprehensiveReport& report, const AnalysisTool& tool);
    
    // Export report
    bool export_json_report(const ComprehensiveReport& report, const std::string& output_file);
    bool export_html_report(const ComprehensiveReport& report, const std::string& output_file);
    
    // Utility methods
    static ProjectSummary analyze_project_structure(const std::string& project_path, 
                                                   const std::vector<std::string>& source_paths);
    static double calculate_quality_score(const ComprehensiveReport::Statistics& stats);
    static std::string determine_quality_rating(double score);
    
private:
    nlohmann::json parse_cppcheck_xml(const std::string& xml_file);
    void calculate_statistics(ComprehensiveReport& report);
    void generate_recommendations(ComprehensiveReport& report);
};

} // namespace gran_azul

// JSON serialization specializations
namespace wip::serialization {

template<>
struct Serializer<nlohmann::json, gran_azul::AnalysisTool> {
    static void to_json(nlohmann::json& j, const gran_azul::AnalysisTool& tool) {
        j["name"] = tool.name;
        j["version"] = tool.version;
        j["description"] = tool.description;
        j["execution_time"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            tool.execution_time.time_since_epoch()).count();
        j["duration_ms"] = tool.duration.count();
        j["success"] = tool.success;
        j["error_message"] = tool.error_message;
        j["results"] = tool.results;
    }
    
    static void from_json(const nlohmann::json& j, gran_azul::AnalysisTool& tool) {
        j.at("name").get_to(tool.name);
        j.at("version").get_to(tool.version);
        j.at("description").get_to(tool.description);
        
        auto time_ms = j.at("execution_time").get<int64_t>();
        tool.execution_time = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(time_ms));
        
        tool.duration = std::chrono::milliseconds(j.at("duration_ms").get<int64_t>());
        j.at("success").get_to(tool.success);
        j.at("error_message").get_to(tool.error_message);
        j.at("results").get_to(tool.results);
    }
};

template<>
struct Serializer<nlohmann::json, gran_azul::ComprehensiveReport> {
    static void to_json(nlohmann::json& j, const gran_azul::ComprehensiveReport& report);
    static void from_json(const nlohmann::json& j, gran_azul::ComprehensiveReport& report);
};

} // namespace wip::serialization