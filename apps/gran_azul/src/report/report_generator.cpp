#include "report_generator.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace gran_azul {

ReportGenerator::ReportGenerator(const std::string& project_base_path)
    : project_base_path_(project_base_path) {
}

ComprehensiveReport ReportGenerator::generate_report(const ProjectSummary& project_info) {
    ComprehensiveReport report;
    
    // Set metadata
    report.generated_at = std::chrono::system_clock::now();
    report.project = project_info;
    
    return report;
}

void ReportGenerator::add_cppcheck_results(ComprehensiveReport& report, const std::string& cppcheck_output_file) {
    AnalysisTool cppcheck_tool;
    cppcheck_tool.name = "Cppcheck";
    cppcheck_tool.description = "Static analysis tool for C/C++ code";
    cppcheck_tool.execution_time = std::chrono::system_clock::now();
    
    try {
        if (std::filesystem::exists(cppcheck_output_file)) {
            // Parse cppcheck XML output
            cppcheck_tool.results = parse_cppcheck_xml(cppcheck_output_file);
            cppcheck_tool.success = true;
            
            // Extract version from results if available
            cppcheck_tool.version = "2.x"; // Default, could be extracted from XML
            
        } else {
            cppcheck_tool.success = false;
            cppcheck_tool.error_message = "Cppcheck output file not found: " + cppcheck_output_file;
        }
    } catch (const std::exception& e) {
        cppcheck_tool.success = false;
        cppcheck_tool.error_message = "Error parsing cppcheck results: " + std::string(e.what());
    }
    
    report.tools.push_back(cppcheck_tool);
    
    // Recalculate statistics
    calculate_statistics(report);
    generate_recommendations(report);
}

void ReportGenerator::add_tool_result(ComprehensiveReport& report, const AnalysisTool& tool) {
    report.tools.push_back(tool);
    calculate_statistics(report);
    generate_recommendations(report);
}

bool ReportGenerator::export_json_report(const ComprehensiveReport& report, const std::string& output_file) {
    try {
        wip::serialization::Serializer<nlohmann::json, ComprehensiveReport> serializer;
        nlohmann::json j;
        serializer.to_json(j, report);
        
        std::ofstream file(output_file);
        if (!file.is_open()) {
            std::cerr << "[REPORT_GENERATOR] Failed to open output file: " << output_file << std::endl;
            return false;
        }
        
        file << j.dump(4) << std::endl;
        file.close();
        
        std::cout << "[REPORT_GENERATOR] JSON report exported to: " << output_file << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[REPORT_GENERATOR] Error exporting JSON report: " << e.what() << std::endl;
        return false;
    }
}

bool ReportGenerator::export_html_report(const ComprehensiveReport& report, const std::string& output_file) {
    try {
        std::ofstream file(output_file);
        if (!file.is_open()) {
            std::cerr << "[REPORT_GENERATOR] Failed to open HTML output file: " << output_file << std::endl;
            return false;
        }
        
        // Generate HTML report
        file << "<!DOCTYPE html>\n";
        file << "<html><head><title>Gran Azul Code Quality Report</title>\n";
        file << "<style>\n";
        file << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
        file << ".header { background-color: #2c3e50; color: white; padding: 20px; border-radius: 8px; }\n";
        file << ".section { margin: 20px 0; padding: 15px; border: 1px solid #ddd; border-radius: 5px; }\n";
        file << ".stats { display: flex; gap: 20px; }\n";
        file << ".stat-box { background: #ecf0f1; padding: 10px; border-radius: 5px; text-align: center; }\n";
        file << ".critical { color: #e74c3c; }\n";
        file << ".major { color: #f39c12; }\n";
        file << ".minor { color: #f1c40f; }\n";
        file << ".info { color: #3498db; }\n";
        file << "</style></head><body>\n";
        
        // Header
        file << "<div class='header'>\n";
        file << "<h1>Gran Azul Code Quality Report</h1>\n";
        file << "<p>Project: " << report.project.name << "</p>\n";
        auto time_t_value = std::chrono::system_clock::to_time_t(report.generated_at);
        file << "<p>Generated: " << std::put_time(std::localtime(&time_t_value), "%Y-%m-%d %H:%M:%S") << "</p>\n";
        file << "</div>\n";
        
        // Statistics
        file << "<div class='section'>\n";
        file << "<h2>Analysis Summary</h2>\n";
        file << "<div class='stats'>\n";
        file << "<div class='stat-box'><h3>" << report.statistics.total_issues << "</h3><p>Total Issues</p></div>\n";
        file << "<div class='stat-box critical'><h3>" << report.statistics.critical_issues << "</h3><p>Critical</p></div>\n";
        file << "<div class='stat-box major'><h3>" << report.statistics.major_issues << "</h3><p>Major</p></div>\n";
        file << "<div class='stat-box minor'><h3>" << report.statistics.minor_issues << "</h3><p>Minor</p></div>\n";
        file << "<div class='stat-box'><h3>" << std::fixed << std::setprecision(1) << report.statistics.quality_score << "</h3><p>Quality Score</p></div>\n";
        file << "</div>\n";
        file << "</div>\n";
        
        // Tools
        for (const auto& tool : report.tools) {
            file << "<div class='section'>\n";
            file << "<h3>" << tool.name << "</h3>\n";
            file << "<p>Status: " << (tool.success ? "Success" : "Failed") << "</p>\n";
            if (!tool.success) {
                file << "<p style='color: red;'>Error: " << tool.error_message << "</p>\n";
            }
            file << "</div>\n";
        }
        
        file << "</body></html>\n";
        file.close();
        
        std::cout << "[REPORT_GENERATOR] HTML report exported to: " << output_file << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[REPORT_GENERATOR] Error exporting HTML report: " << e.what() << std::endl;
        return false;
    }
}

ProjectSummary ReportGenerator::analyze_project_structure(const std::string& project_path, 
                                                        const std::vector<std::string>& source_paths) {
    ProjectSummary summary;
    summary.root_path = project_path;
    summary.created_at = std::chrono::system_clock::now();
    summary.last_modified = std::chrono::system_clock::now();
    summary.source_paths = source_paths;
    
    // Analyze files
    summary.total_files = 0;
    summary.lines_of_code = 0;
    
    for (const auto& source_path : source_paths) {
        if (std::filesystem::exists(source_path)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(source_path)) {
                if (entry.is_regular_file()) {
                    std::string extension = entry.path().extension().string();
                    if (extension == ".cpp" || extension == ".h" || extension == ".hpp" || 
                        extension == ".c" || extension == ".cc" || extension == ".cxx") {
                        summary.total_files++;
                        
                        // Count lines (simplified)
                        std::ifstream file(entry.path());
                        summary.lines_of_code += std::count(
                            std::istreambuf_iterator<char>(file),
                            std::istreambuf_iterator<char>(), '\n');
                        
                        // Track extensions
                        if (std::find(summary.file_extensions.begin(), 
                                    summary.file_extensions.end(), extension) == summary.file_extensions.end()) {
                            summary.file_extensions.push_back(extension);
                        }
                    }
                }
            }
        }
    }
    
    return summary;
}

double ReportGenerator::calculate_quality_score(const ComprehensiveReport::Statistics& stats) {
    if (stats.total_issues == 0) return 100.0;
    
    // Weighted scoring: critical = -10, major = -5, minor = -2, info = -1
    double penalty = (stats.critical_issues * 10) + 
                    (stats.major_issues * 5) + 
                    (stats.minor_issues * 2) + 
                    (stats.info_issues * 1);
    
    // Scale based on lines of code (penalty per 1000 lines)
    double score = 100.0 - (penalty / 10.0);
    
    return std::max(0.0, std::min(100.0, score));
}

std::string ReportGenerator::determine_quality_rating(double score) {
    if (score >= 90) return "A";
    if (score >= 80) return "B";
    if (score >= 70) return "C";
    if (score >= 60) return "D";
    return "E";
}

nlohmann::json ReportGenerator::parse_cppcheck_xml(const std::string& xml_file) {
    nlohmann::json results;
    results["issues"] = nlohmann::json::array();
    results["file"] = xml_file;
    
    try {
        std::ifstream file(xml_file);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + xml_file);
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        // Simple XML parsing for cppcheck format
        // This is a simplified parser - in production you'd use a proper XML library
        size_t pos = 0;
        while ((pos = content.find("<error", pos)) != std::string::npos) {
            size_t end_pos = content.find("/>", pos);
            if (end_pos != std::string::npos) {
                std::string error_tag = content.substr(pos, end_pos - pos + 2);
                
                nlohmann::json issue;
                
                // Extract attributes (simplified)
                auto extract_attr = [&](const std::string& attr) -> std::string {
                    std::string pattern = attr + "=\"";
                    size_t attr_pos = error_tag.find(pattern);
                    if (attr_pos != std::string::npos) {
                        attr_pos += pattern.length();
                        size_t end_attr = error_tag.find("\"", attr_pos);
                        if (end_attr != std::string::npos) {
                            return error_tag.substr(attr_pos, end_attr - attr_pos);
                        }
                    }
                    return "";
                };
                
                issue["id"] = extract_attr("id");
                issue["severity"] = extract_attr("severity");
                issue["msg"] = extract_attr("msg");
                issue["file"] = extract_attr("file");
                issue["line"] = extract_attr("line");
                issue["column"] = extract_attr("column");
                
                results["issues"].push_back(issue);
            }
            pos = end_pos + 1;
        }
        
    } catch (const std::exception& e) {
        results["parse_error"] = e.what();
    }
    
    return results;
}

void ReportGenerator::calculate_statistics(ComprehensiveReport& report) {
    report.statistics = {}; // Reset
    
    for (const auto& tool : report.tools) {
        if (!tool.success || !tool.results.contains("issues")) continue;
        
        for (const auto& issue : tool.results["issues"]) {
            if (!issue.contains("severity")) continue;
            
            std::string severity = issue["severity"];
            report.statistics.total_issues++;
            
            if (severity == "error") {
                report.statistics.critical_issues++;
            } else if (severity == "warning") {
                report.statistics.major_issues++;
            } else if (severity == "style" || severity == "performance") {
                report.statistics.minor_issues++;
            } else {
                report.statistics.info_issues++;
            }
        }
    }
    
    report.statistics.quality_score = calculate_quality_score(report.statistics);
    report.statistics.quality_rating = determine_quality_rating(report.statistics.quality_score);
}

void ReportGenerator::generate_recommendations(ComprehensiveReport& report) {
    report.recommendations.clear();
    report.warnings.clear();
    
    if (report.statistics.critical_issues > 0) {
        report.recommendations.push_back("Address " + std::to_string(report.statistics.critical_issues) + 
                                       " critical issues immediately");
    }
    
    if (report.statistics.major_issues > 10) {
        report.recommendations.push_back("Consider refactoring to reduce " + 
                                       std::to_string(report.statistics.major_issues) + " major issues");
    }
    
    if (report.statistics.quality_score < 70) {
        report.warnings.push_back("Code quality score is below recommended threshold (70)");
        report.recommendations.push_back("Implement code review processes and automated quality checks");
    }
    
    // Overall assessment
    if (report.statistics.quality_score >= 90) {
        report.overall_assessment = "Excellent code quality";
    } else if (report.statistics.quality_score >= 80) {
        report.overall_assessment = "Good code quality with minor improvements needed";
    } else if (report.statistics.quality_score >= 70) {
        report.overall_assessment = "Acceptable code quality, consider improvements";
    } else {
        report.overall_assessment = "Code quality needs significant improvement";
    }
}

} // namespace gran_azul

// JSON serialization implementation
namespace wip::serialization {

void Serializer<nlohmann::json, gran_azul::ComprehensiveReport>::to_json(nlohmann::json& j, const gran_azul::ComprehensiveReport& report) {
    j["report_version"] = report.report_version;
    j["generated_at"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        report.generated_at.time_since_epoch()).count();
    j["generator_name"] = report.generator_name;
    j["generator_version"] = report.generator_version;
    
    j["project"]["name"] = report.project.name;
    j["project"]["root_path"] = report.project.root_path;
    j["project"]["project_file"] = report.project.project_file;
    j["project"]["total_files"] = report.project.total_files;
    j["project"]["lines_of_code"] = report.project.lines_of_code;
    j["project"]["file_extensions"] = report.project.file_extensions;
    j["project"]["source_paths"] = report.project.source_paths;
    
    j["statistics"]["total_issues"] = report.statistics.total_issues;
    j["statistics"]["critical_issues"] = report.statistics.critical_issues;
    j["statistics"]["major_issues"] = report.statistics.major_issues;
    j["statistics"]["minor_issues"] = report.statistics.minor_issues;
    j["statistics"]["info_issues"] = report.statistics.info_issues;
    j["statistics"]["quality_score"] = report.statistics.quality_score;
    j["statistics"]["quality_rating"] = report.statistics.quality_rating;
    
    j["tools"] = nlohmann::json::array();
    for (const auto& tool : report.tools) {
        nlohmann::json tool_json;
        Serializer<nlohmann::json, gran_azul::AnalysisTool>::to_json(tool_json, tool);
        j["tools"].push_back(tool_json);
    }
    j["recommendations"] = report.recommendations;
    j["warnings"] = report.warnings;
    j["overall_assessment"] = report.overall_assessment;
}

void Serializer<nlohmann::json, gran_azul::ComprehensiveReport>::from_json(const nlohmann::json& j, gran_azul::ComprehensiveReport& report) {
    j.at("report_version").get_to(report.report_version);
    
    auto time_ms = j.at("generated_at").get<int64_t>();
    report.generated_at = std::chrono::system_clock::time_point(std::chrono::milliseconds(time_ms));
    
    j.at("generator_name").get_to(report.generator_name);
    j.at("generator_version").get_to(report.generator_version);
    
    // Project info
    const auto& proj = j.at("project");
    proj.at("name").get_to(report.project.name);
    proj.at("root_path").get_to(report.project.root_path);
    proj.at("project_file").get_to(report.project.project_file);
    proj.at("total_files").get_to(report.project.total_files);
    proj.at("lines_of_code").get_to(report.project.lines_of_code);
    proj.at("file_extensions").get_to(report.project.file_extensions);
    proj.at("source_paths").get_to(report.project.source_paths);
    
    // Statistics
    const auto& stats = j.at("statistics");
    stats.at("total_issues").get_to(report.statistics.total_issues);
    stats.at("critical_issues").get_to(report.statistics.critical_issues);
    stats.at("major_issues").get_to(report.statistics.major_issues);
    stats.at("minor_issues").get_to(report.statistics.minor_issues);
    stats.at("info_issues").get_to(report.statistics.info_issues);
    stats.at("quality_score").get_to(report.statistics.quality_score);
    stats.at("quality_rating").get_to(report.statistics.quality_rating);
    
    report.tools.clear();
    for (const auto& tool_json : j.at("tools")) {
        gran_azul::AnalysisTool tool;
        Serializer<nlohmann::json, gran_azul::AnalysisTool>::from_json(tool_json, tool);
        report.tools.push_back(tool);
    }
    j.at("recommendations").get_to(report.recommendations);
    j.at("warnings").get_to(report.warnings);
    j.at("overall_assessment").get_to(report.overall_assessment);
}

} // namespace wip::serialization