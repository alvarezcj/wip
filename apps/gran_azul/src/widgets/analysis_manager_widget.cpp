#include "analysis_manager_widget.h"
#include "../project/project_config.h"
#include <imgui.h>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <sstream>

namespace gran_azul {
namespace widgets {

AnalysisManagerWidget::AnalysisManagerWidget() {
    // Create analysis engine with all available tools
    analysis_engine_ = wip::analysis::AnalysisEngineFactory::create_default_engine();
    
    // Initialize tool enabled states
    tool_enabled_["cppcheck"] = true;
    tool_enabled_["clang-tidy"] = false; // Disabled by default, user can enable
    
    // Initialize selected tools based on enabled state
    for (const auto& [name, is_enabled] : tool_enabled_) {
        if (is_enabled) {
            selected_tools_.push_back(name);
        }
    }
    
    std::cout << "[ANALYSIS_MANAGER] Widget initialized with " 
              << analysis_engine_->get_registered_tools().size() << " analysis tools\n";
    std::cout << "[ANALYSIS_MANAGER] Selected tools: ";
    for (const auto& tool : selected_tools_) {
        std::cout << tool << " ";
    }
    std::cout << "\n";
}

void AnalysisManagerWidget::update(float delta_time) {
    // Update any time-based UI animations here if needed
}

void AnalysisManagerWidget::render() {
    if (ImGui::Begin("Analysis Configuration", nullptr, ImGuiWindowFlags_None)) {
        if (!project_loaded_) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No project loaded");
            ImGui::Text("Please create or load a project to configure analysis.");
            ImGui::End();
            return;
        }
        
        // Tool selection section
        ImGui::Text("Analysis Tools");
        ImGui::Separator();
        render_tool_selection();
        
        ImGui::Spacing();
        
        // Common configuration
        ImGui::Text("Common Configuration");
        ImGui::Separator();
        render_common_configuration();
        
        ImGui::Spacing();
        
        // Tool-specific configurations
        if (tool_enabled_["cppcheck"]) {
            ImGui::Text("Cppcheck Configuration");
            ImGui::Separator();
            render_cppcheck_configuration();
            ImGui::Spacing();
        }
        
        if (tool_enabled_["clang-tidy"]) {
            ImGui::Text("Clang-Tidy Configuration");
            ImGui::Separator();
            render_clang_tidy_configuration();
            ImGui::Spacing();
        }
        
        // Analysis controls
        ImGui::Separator();
        render_analysis_controls();
    }
    ImGui::End();
}

void AnalysisManagerWidget::render_tool_selection() {
    auto available_tools = analysis_engine_->get_available_tools();
    
    if (available_tools.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No analysis tools available");
        ImGui::Text("Please install cppcheck and/or clang-tidy");
        return;
    }
    
    // Show available tools with checkboxes
    for (const auto& tool : available_tools) {
        bool enabled = tool_enabled_[tool];
        if (ImGui::Checkbox(tool.c_str(), &enabled)) {
            tool_enabled_[tool] = enabled;
            
            // Update selected tools list
            selected_tools_.clear();
            for (const auto& [name, is_enabled] : tool_enabled_) {
                if (is_enabled) {
                    selected_tools_.push_back(name);
                }
            }
        }
        
        // Show tool status
        ImGui::SameLine();
        auto* tool_instance = analysis_engine_->get_tool(tool);
        if (tool_instance && tool_instance->is_available()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "(Available)");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "(Not Available)");
        }
    }
}

void AnalysisManagerWidget::render_common_configuration() {
    // Source path
    ImGui::Text("Source Path:");
    ImGui::InputText("##source_path", source_path_, sizeof(source_path_));
    ImGui::SameLine();
    if (ImGui::Button("Browse##source")) {
        if (directory_callback_) {
            std::string selected = directory_callback_();
            if (!selected.empty()) {
                strncpy(source_path_, selected.c_str(), sizeof(source_path_) - 1);
                source_path_[sizeof(source_path_) - 1] = '\0';
                
                // Notify that configuration changed so project can be saved
                if (config_changed_callback_) {
                    config_changed_callback_();
                }
            }
        }
    }
    
    // Output directory
    ImGui::Text("Output Directory:");
    ImGui::InputText("##output_dir", output_directory_, sizeof(output_directory_));
    ImGui::SameLine();
    if (ImGui::Button("Browse##output")) {
        if (directory_callback_) {
            std::string selected = directory_callback_();
            if (!selected.empty()) {
                strncpy(output_directory_, selected.c_str(), sizeof(output_directory_) - 1);
                output_directory_[sizeof(output_directory_) - 1] = '\0';
                
                // Notify that configuration changed so project can be saved
                if (config_changed_callback_) {
                    config_changed_callback_();
                }
            }
        }
    }
}

void AnalysisManagerWidget::render_cppcheck_configuration() {
    // Enable checks section
    if (ImGui::CollapsingHeader("Enabled Checks", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable All", &cppcheck_enable_all_);
        
        if (!cppcheck_enable_all_) {
            ImGui::Indent();
            ImGui::Checkbox("Warnings", &cppcheck_enable_warning_);
            ImGui::Checkbox("Style", &cppcheck_enable_style_);
            ImGui::Checkbox("Performance", &cppcheck_enable_performance_);
            ImGui::Checkbox("Portability", &cppcheck_enable_portability_);
            ImGui::Checkbox("Information", &cppcheck_enable_information_);
            ImGui::Unindent();
        }
    }
    
    // C++ Standard
    if (ImGui::CollapsingHeader("Standards & Performance")) {
        const char* cpp_standards[] = {"C++03", "C++11", "C++14", "C++17", "C++20"};
        ImGui::Combo("C++ Standard", &cppcheck_cpp_standard_, cpp_standards, IM_ARRAYSIZE(cpp_standards));
        
        ImGui::SliderInt("Job Count", &cppcheck_job_count_, 1, 16);
    }
}

void AnalysisManagerWidget::render_clang_tidy_configuration() {
    // Checks configuration
    if (ImGui::CollapsingHeader("Check Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Checks (comma-separated patterns):");
        if (ImGui::InputText("##clang_tidy_checks", clang_tidy_checks_input_, sizeof(clang_tidy_checks_input_))) {
            // Parse the input when it changes
            clang_tidy_checks_ = parse_clang_tidy_checks();
        }
        ImGui::TextWrapped("Examples: bugprone-*, modernize-*, -readability-magic-numbers");
    }
    
    // Options
    if (ImGui::CollapsingHeader("Options")) {
        ImGui::Checkbox("Fix Errors", &clang_tidy_fix_errors_);
        ImGui::Checkbox("Enable Header Filter", &clang_tidy_header_filter_);
    }
}

void AnalysisManagerWidget::render_analysis_controls() {
    // Check if any tools are enabled
    bool can_analyze = false;
    for (const auto& [tool, enabled] : tool_enabled_) {
        if (enabled && analysis_engine_->is_tool_available(tool)) {
            can_analyze = true;
            break;
        }
    }
    
    if (!can_analyze) {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No tools enabled or available");
        return;
    }
    
    // Run analysis button
    if (ImGui::Button("Run Analysis", ImVec2(120, 30))) {
        if (analysis_callback_) {
            // Configure tools before running analysis
            for (const auto& tool : selected_tools_) {
                if (tool == "cppcheck") {
                    configure_cppcheck_tool();
                } else if (tool == "clang-tidy") {
                    configure_clang_tidy_tool();
                }
            }
            
            auto request = build_analysis_request();
            std::cout << "[ANALYSIS_MANAGER] Running analysis with " << selected_tools_.size() 
                      << " tools, source: " << request.source_path << ", output: " << request.output_file << "\n";
            analysis_callback_(selected_tools_, request);
        }
    }
    
    ImGui::SameLine();
    
    // Version check buttons
    if (ImGui::Button("Check Tool Versions", ImVec2(140, 30))) {
        for (const auto& tool : selected_tools_) {
            if (version_callback_) {
                version_callback_(tool);
            }
        }
    }
    
    // Show selected tools
    if (!selected_tools_.empty()) {
        ImGui::Text("Selected tools: ");
        ImGui::SameLine();
        for (size_t i = 0; i < selected_tools_.size(); ++i) {
            if (i > 0) ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", selected_tools_[i].c_str());
            if (i < selected_tools_.size() - 1) {
                ImGui::SameLine();
                ImGui::Text(",");
            }
        }
    }
}

wip::analysis::AnalysisRequest AnalysisManagerWidget::build_analysis_request() const {
    wip::analysis::AnalysisRequest request;
    
    request.source_path = std::string(source_path_);
    
    // Create tool-specific output files
    std::filesystem::path output_dir(output_directory_);
    
    // For now, we'll use a single output file - the analysis engine can handle multiple tools
    request.output_file = (output_dir / "analysis_results.xml").string();
    
    return request;
}

void AnalysisManagerWidget::configure_cppcheck_tool() {
    auto* cppcheck_tool = analysis_engine_->get_tool("cppcheck");
    if (!cppcheck_tool) return;
    
    auto config = std::make_unique<wip::analysis::tools::CppcheckConfig>();
    
    // Configure based on UI settings
    config->tool_name = "cppcheck";
    config->source_path = std::string(source_path_);
    config->output_file = (std::filesystem::path(output_directory_) / "cppcheck_results.xml").string();
    
    config->enable_all = cppcheck_enable_all_;
    config->enable_warning = cppcheck_enable_warning_;
    config->enable_style = cppcheck_enable_style_;
    config->enable_performance = cppcheck_enable_performance_;
    config->enable_portability = cppcheck_enable_portability_;
    config->enable_information = cppcheck_enable_information_;
    
    // Map UI enum to config enum
    config->cpp_standard = static_cast<wip::analysis::tools::CppcheckConfig::CppStandard>(cppcheck_cpp_standard_);
    config->job_count = cppcheck_job_count_;
    
    analysis_engine_->set_tool_configuration("cppcheck", std::move(config));
}

void AnalysisManagerWidget::configure_clang_tidy_tool() {
    auto* clang_tidy_tool = analysis_engine_->get_tool("clang-tidy");
    if (!clang_tidy_tool) return;
    
    auto config = std::make_unique<wip::analysis::tools::ClangTidyConfig>();
    
    config->tool_name = "clang-tidy";
    config->source_path = std::string(source_path_);
    config->output_file = (std::filesystem::path(output_directory_) / "clang_tidy_results.yaml").string();
    
    config->checks = clang_tidy_checks_;
    config->fix_errors = clang_tidy_fix_errors_;
    config->header_filter_regex_enabled = clang_tidy_header_filter_;
    if (clang_tidy_header_filter_) {
        config->header_filter_regex = ".*";
    }
    
    analysis_engine_->set_tool_configuration("clang-tidy", std::move(config));
}

std::vector<std::string> AnalysisManagerWidget::parse_clang_tidy_checks() const {
    std::vector<std::string> checks;
    std::string input(clang_tidy_checks_input_);
    
    std::stringstream ss(input);
    std::string check;
    
    while (std::getline(ss, check, ',')) {
        // Trim whitespace
        check.erase(0, check.find_first_not_of(" \t"));
        check.erase(check.find_last_not_of(" \t") + 1);
        
        if (!check.empty()) {
            checks.push_back(check);
        }
    }
    
    return checks;
}

void AnalysisManagerWidget::set_project_base_path(const std::string& path) {
    project_base_path_ = path;
    
    // Update default paths relative to project - but only if they haven't been set yet
    if (!path.empty()) {
        std::filesystem::path project_path(path);
        
        // Only set default source path if it's currently empty or default
        if (strlen(source_path_) == 0 || strcmp(source_path_, "./") == 0) {
            std::string default_source = project_path.string();
            strncpy(source_path_, default_source.c_str(), sizeof(source_path_) - 1);
            source_path_[sizeof(source_path_) - 1] = '\0';
        }
        
        // Set default output directory
        std::string default_output = (project_path / "analysis_results").string();
        strncpy(output_directory_, default_output.c_str(), sizeof(output_directory_) - 1);
        output_directory_[sizeof(output_directory_) - 1] = '\0';
    }
}

void AnalysisManagerWidget::load_from_project_config(const ProjectConfig& config) {
    // Load source path
    strncpy(source_path_, config.analysis.source_path.c_str(), sizeof(source_path_) - 1);
    source_path_[sizeof(source_path_) - 1] = '\0';
    
    // Load output directory (derive from output file)
    std::filesystem::path output_file(config.analysis.output_file);
    std::string output_dir = output_file.parent_path().string();
    if (output_dir.empty()) output_dir = "./analysis_results";
    
    strncpy(output_directory_, output_dir.c_str(), sizeof(output_directory_) - 1);
    output_directory_[sizeof(output_directory_) - 1] = '\0';
    
    // Load cppcheck settings
    cppcheck_enable_all_ = config.analysis.enable_all;
    cppcheck_enable_warning_ = config.analysis.enable_warning;
    cppcheck_enable_style_ = config.analysis.enable_style;
    cppcheck_enable_performance_ = config.analysis.enable_performance;
    cppcheck_enable_portability_ = config.analysis.enable_portability;
    cppcheck_enable_information_ = config.analysis.enable_information;
    cppcheck_cpp_standard_ = config.analysis.cpp_standard;
    cppcheck_job_count_ = config.analysis.job_count;
}

void AnalysisManagerWidget::save_to_project_config(ProjectConfig& config) const {
    // Save common settings
    config.analysis.source_path = std::string(source_path_);
    config.analysis.output_file = (std::filesystem::path(output_directory_) / "analysis_results.xml").string();
    
    // Save cppcheck settings
    config.analysis.enable_all = cppcheck_enable_all_;
    config.analysis.enable_warning = cppcheck_enable_warning_;
    config.analysis.enable_style = cppcheck_enable_style_;
    config.analysis.enable_performance = cppcheck_enable_performance_;
    config.analysis.enable_portability = cppcheck_enable_portability_;
    config.analysis.enable_information = cppcheck_enable_information_;
    config.analysis.cpp_standard = cppcheck_cpp_standard_;
    config.analysis.job_count = cppcheck_job_count_;
}

bool AnalysisManagerWidget::has_available_tools() const {
    auto available_tools = analysis_engine_->get_available_tools();
    return !available_tools.empty();
}

std::vector<std::string> AnalysisManagerWidget::get_available_tools() const {
    return analysis_engine_->get_available_tools();
}

} // namespace widgets
} // namespace gran_azul