#include "analysis_config_widget.h"
#include <imgui.h>
#include <filesystem>
#include <iostream>

namespace gran_azul::widgets {

AnalysisConfigWidget::AnalysisConfigWidget() {
    // Constructor implementation
}

void AnalysisConfigWidget::render(gran_azul::ProjectConfig::AnalysisConfig& config) {
    if (ImGui::BeginTabBar("AnalysisConfigTabs", ImGuiTabBarFlags_None)) {
        render_tools_tab(config);
        render_cppcheck_tab(config);
        render_clang_tidy_tab(config);
        render_common_tab(config);
        ImGui::EndTabBar();
    }
}

void AnalysisConfigWidget::update(float delta_time) {
    // Update logic if needed
}

void AnalysisConfigWidget::draw() {
    if (!ImGui::Begin("Analysis Configuration")) {
        ImGui::End();
        return;
    }
    
    // Main tab bar
    if (ImGui::BeginTabBar("AnalysisConfigTabs")) {
        
        if (ImGui::BeginTabItem("Tools")) {
            render_tool_selection_tab();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Cppcheck")) {
            render_cppcheck_tab();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Clang-Tidy")) {
            render_clang_tidy_tab();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Common")) {
            render_common_tab();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void AnalysisConfigWidget::render_tool_selection_tab() {
    ImGui::Text("Select Analysis Tools");
    ImGui::Separator();
    
    // Tool availability status
    ImGui::Text("Tool Availability:");
    ImGui::Indent();
    
    bool cppcheck_available = is_tool_available("cppcheck");
    bool clang_tidy_available = is_tool_available("clang-tidy");
    
    ImGui::TextColored(cppcheck_available ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), 
                      "Cppcheck: %s", cppcheck_available ? "Available" : "Not Found");
    
    if (cppcheck_available) {
        ImGui::SameLine();
        if (ImGui::SmallButton("Check Version##cppcheck")) {
            if (on_version_check_) on_version_check_("cppcheck");
        }
    }
    
    ImGui::TextColored(clang_tidy_available ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1),
                      "Clang-Tidy: %s", clang_tidy_available ? "Available" : "Not Found");
    
    if (clang_tidy_available) {
        ImGui::SameLine();
        if (ImGui::SmallButton("Check Version##clang-tidy")) {
            if (on_version_check_) on_version_check_("clang-tidy");
        }
    }
    
    ImGui::Unindent();
    ImGui::Spacing();
    
    // Tool selection
    ImGui::Text("Enable Tools:");
    ImGui::Separator();
    
    bool config_changed = false;
    
    if (ImGui::Checkbox("Enable Cppcheck", &config_.enable_cppcheck)) {
        config_changed = true;
    }
    if (!cppcheck_available) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "(Tool not found)");
    }
    
    if (ImGui::Checkbox("Enable Clang-Tidy", &config_.enable_clang_tidy)) {
        config_changed = true;
    }
    if (!clang_tidy_available) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "(Tool not found)");
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Analysis controls
    ImGui::Text("Analysis Control:");
    
    if (!project_loaded_) {
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "No project loaded");
    } else {
        std::vector<std::string> enabled_tools = get_enabled_tools();
        
        if (enabled_tools.empty()) {
            ImGui::TextColored(ImVec4(1,0.5f,0,1), "No tools enabled");
        } else {
            ImGui::Text("Enabled tools: ");
            for (size_t i = 0; i < enabled_tools.size(); ++i) {
                if (i > 0) ImGui::SameLine();
                ImGui::Text("%s", enabled_tools[i].c_str());
                if (i < enabled_tools.size() - 1) {
                    ImGui::SameLine();
                    ImGui::Text(",");
                }
            }
            
            ImGui::Spacing();
            
            if (ImGui::Button("Run Analysis", ImVec2(120, 0))) {
                if (on_analysis_requested_) {
                    on_analysis_requested_(enabled_tools);
                }
            }
        }
    }
    
    if (config_changed) {
        trigger_config_changed();
    }
}

void AnalysisConfigWidget::render_cppcheck_tab() {
    if (!config_.enable_cppcheck) {
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "Cppcheck is disabled. Enable it in the Tools tab.");
        return;
    }
    
    bool config_changed = false;
    
    ImGui::Text("Cppcheck Configuration");
    ImGui::Separator();
    
    // Basic options
    ImGui::Text("Check Categories:");
    if (ImGui::Checkbox("All checks", &config_.cppcheck.enable_all)) config_changed = true;
    if (ImGui::Checkbox("Warnings", &config_.cppcheck.enable_warning)) config_changed = true;
    if (ImGui::Checkbox("Style", &config_.cppcheck.enable_style)) config_changed = true;
    if (ImGui::Checkbox("Performance", &config_.cppcheck.enable_performance)) config_changed = true;
    if (ImGui::Checkbox("Portability", &config_.cppcheck.enable_portability)) config_changed = true;
    if (ImGui::Checkbox("Information", &config_.cppcheck.enable_information)) config_changed = true;
    if (ImGui::Checkbox("Unused functions", &config_.cppcheck.enable_unused_function)) config_changed = true;
    if (ImGui::Checkbox("Missing includes", &config_.cppcheck.enable_missing_include)) config_changed = true;
    
    ImGui::Spacing();
    
    // Analysis level
    ImGui::Text("Analysis Level:");
    const char* check_levels[] = { "Normal", "Exhaustive" };
    if (ImGui::Combo("Check Level", &config_.cppcheck.check_level, check_levels, 2)) config_changed = true;
    
    if (ImGui::Checkbox("Inconclusive results", &config_.cppcheck.inconclusive)) config_changed = true;
    if (ImGui::Checkbox("Verbose output", &config_.cppcheck.verbose)) config_changed = true;
    
    ImGui::Spacing();
    
    // Performance settings
    ImGui::Text("Performance:");
    if (ImGui::SliderInt("Job count", &config_.cppcheck.job_count, 1, 16)) config_changed = true;
    if (ImGui::Checkbox("Quiet mode", &config_.cppcheck.quiet)) config_changed = true;
    
    ImGui::Spacing();
    
    // Advanced options toggle
    if (ImGui::Button(show_advanced_cppcheck_ ? "Hide Advanced" : "Show Advanced")) {
        show_advanced_cppcheck_ = !show_advanced_cppcheck_;
    }
    
    if (show_advanced_cppcheck_) {
        ImGui::Spacing();
        render_cppcheck_advanced_options();
        if (config_changed) config_changed = true; // This is redundant but clear
    }
    
    if (config_changed) {
        trigger_config_changed();
    }
}

void AnalysisConfigWidget::render_cppcheck_advanced_options() {
    ImGui::Text("Advanced Cppcheck Options");
    ImGui::Separator();
    
    bool config_changed = false;
    
    // Suppressions
    ImGui::Text("Suppressions:");
    if (ImGui::Checkbox("Suppress unused functions", &config_.cppcheck.suppress_unused_function)) config_changed = true;
    if (ImGui::Checkbox("Suppress missing system includes", &config_.cppcheck.suppress_missing_include_system)) config_changed = true;
    if (ImGui::Checkbox("Suppress missing includes", &config_.cppcheck.suppress_missing_include)) config_changed = true;
    if (ImGui::Checkbox("Suppress duplicate conditionals", &config_.cppcheck.suppress_duplicate_conditional)) config_changed = true;
    
    ImGui::Spacing();
    
    // Libraries
    ImGui::Text("Libraries:");
    if (ImGui::Checkbox("Use POSIX library", &config_.cppcheck.use_posix_library)) config_changed = true;
    if (ImGui::Checkbox("Use MISRA addon", &config_.cppcheck.use_misra_addon)) config_changed = true;
    
    if (config_changed) {
        trigger_config_changed();
    }
}

void AnalysisConfigWidget::render_clang_tidy_tab() {
    if (!config_.enable_clang_tidy) {
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "Clang-Tidy is disabled. Enable it in the Tools tab.");
        return;
    }
    
    bool config_changed = false;
    
    ImGui::Text("Clang-Tidy Configuration");
    ImGui::Separator();
    
    render_clang_tidy_checks();
    
    ImGui::Spacing();
    ImGui::Separator();
    
    render_clang_tidy_options();
    
    if (config_changed) {
        trigger_config_changed();
    }
}

void AnalysisConfigWidget::render_clang_tidy_checks() {
    bool config_changed = false;
    
    ImGui::Text("Check Categories:");
    
    if (ImGui::Checkbox("Bugprone checks", &config_.clang_tidy.enable_bugprone_checks)) config_changed = true;
    if (ImGui::Checkbox("Performance checks", &config_.clang_tidy.enable_performance_checks)) config_changed = true;
    if (ImGui::Checkbox("Modernize checks", &config_.clang_tidy.enable_modernize_checks)) config_changed = true;
    if (ImGui::Checkbox("Readability checks", &config_.clang_tidy.enable_readability_checks)) config_changed = true;
    if (ImGui::Checkbox("C++ Core Guidelines", &config_.clang_tidy.enable_cppcoreguidelines_checks)) config_changed = true;
    if (ImGui::Checkbox("Misc checks", &config_.clang_tidy.enable_misc_checks)) config_changed = true;
    if (ImGui::Checkbox("CERT checks", &config_.clang_tidy.enable_cert_checks)) config_changed = true;
    
    ImGui::Spacing();
    ImGui::Text("Disabled Checks:");
    if (ImGui::Checkbox("Disable magic numbers warnings", &config_.clang_tidy.disable_magic_numbers)) config_changed = true;
    if (ImGui::Checkbox("Disable uppercase literal suffix warnings", &config_.clang_tidy.disable_uppercase_literal_suffix)) config_changed = true;
    
    if (config_changed) {
        trigger_config_changed();
    }
}

void AnalysisConfigWidget::render_clang_tidy_options() {
    bool config_changed = false;
    
    ImGui::Text("Analysis Options:");
    
    if (ImGui::Checkbox("Use color output", &config_.clang_tidy.use_color)) config_changed = true;
    if (ImGui::Checkbox("Export fixes", &config_.clang_tidy.export_fixes)) config_changed = true;
    if (ImGui::Checkbox("Fix errors", &config_.clang_tidy.fix_errors)) config_changed = true;
    if (ImGui::Checkbox("Fix notes", &config_.clang_tidy.fix_notes)) config_changed = true;
    
    ImGui::Spacing();
    
    // Format style
    ImGui::Text("Format Style:");
    const char* format_styles[] = { "file", "llvm", "google", "chromium", "mozilla", "webkit" };
    int format_style_index = 0;
    for (int i = 0; i < 6; ++i) {
        if (config_.clang_tidy.format_style == format_styles[i]) {
            format_style_index = i;
            break;
        }
    }
    if (ImGui::Combo("Format Style", &format_style_index, format_styles, 6)) {
        config_.clang_tidy.format_style = format_styles[format_style_index];
        config_changed = true;
    }
    
    ImGui::Spacing();
    
    // Header filter
    if (ImGui::Checkbox("Enable header filter", &config_.clang_tidy.header_filter_regex_enabled)) config_changed = true;
    
    if (config_.clang_tidy.header_filter_regex_enabled) {
        char header_filter_buf[256];
        strncpy(header_filter_buf, config_.clang_tidy.header_filter_regex.c_str(), sizeof(header_filter_buf) - 1);
        header_filter_buf[sizeof(header_filter_buf) - 1] = '\0';
        
        if (ImGui::InputText("Header Filter Regex", header_filter_buf, sizeof(header_filter_buf))) {
            config_.clang_tidy.header_filter_regex = header_filter_buf;
            config_changed = true;
        }
    }
    
    if (ImGui::Checkbox("Include system headers", &config_.clang_tidy.system_headers)) config_changed = true;
    
    ImGui::Spacing();
    
    // Config file
    ImGui::Text("Configuration File (optional):");
    char config_file_buf[512];
    strncpy(config_file_buf, config_.clang_tidy.config_file.c_str(), sizeof(config_file_buf) - 1);
    config_file_buf[sizeof(config_file_buf) - 1] = '\0';
    
    if (ImGui::InputText("Config File Path", config_file_buf, sizeof(config_file_buf))) {
        config_.clang_tidy.config_file = config_file_buf;
        config_changed = true;
    }
    
    if (config_changed) {
        trigger_config_changed();
    }
}

void AnalysisConfigWidget::render_common_tab() {
    bool config_changed = false;
    
    ImGui::Text("Common Analysis Settings");
    ImGui::Separator();
    
    // Source path
    if (input_text_project_relative("Source Path", config_.source_path, "Relative path from project root")) {
        config_changed = true;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Browse##source")) {
        if (on_directory_selection_) {
            std::string selected = on_directory_selection_();
            if (!selected.empty()) {
                config_.source_path = selected;
                config_changed = true;
            }
        }
    }
    
    // Output file
    char output_file_buf[512];
    strncpy(output_file_buf, config_.output_file.c_str(), sizeof(output_file_buf) - 1);
    output_file_buf[sizeof(output_file_buf) - 1] = '\0';
    
    if (ImGui::InputText("Output File", output_file_buf, sizeof(output_file_buf))) {
        config_.output_file = output_file_buf;
        config_changed = true;
    }
    
    // Build directory
    if (input_text_project_relative("Build Directory", config_.build_dir, "Directory for build files")) {
        config_changed = true;
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Standards
    ImGui::Text("Language Standards:");
    const char* cpp_standards[] = { "C++03", "C++11", "C++14", "C++17", "C++20" };
    if (ImGui::Combo("C++ Standard", &config_.cpp_standard, cpp_standards, 5)) config_changed = true;
    
    const char* platforms[] = { "Unix 32-bit", "Unix 64-bit", "Windows 32-bit", "Windows 64-bit" };
    if (ImGui::Combo("Platform", &config_.platform, platforms, 4)) config_changed = true;
    
    if (config_changed) {
        trigger_config_changed();
    }
}

void AnalysisConfigWidget::set_config(const ProjectConfig::AnalysisConfig& config) {
    config_ = config;
}

std::vector<std::string> AnalysisConfigWidget::get_enabled_tools() const {
    std::vector<std::string> tools;
    
    if (config_.enable_cppcheck && is_tool_available("cppcheck")) {
        tools.push_back("cppcheck");
    }
    
    if (config_.enable_clang_tidy && is_tool_available("clang-tidy")) {
        tools.push_back("clang-tidy");
    }
    
    return tools;
}

void AnalysisConfigWidget::trigger_config_changed() {
    if (on_config_changed_) {
        on_config_changed_(config_);
    }
}

bool AnalysisConfigWidget::is_tool_available(const std::string& tool_name) const {
    // Simple check - try to run tool with --version
    std::string command = tool_name + " --version > /dev/null 2>&1";
    return system(command.c_str()) == 0;
}

bool AnalysisConfigWidget::input_text_project_relative(const char* label, std::string& path, const char* hint) {
    char buf[512];
    strncpy(buf, path.c_str(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    
    bool changed = false;
    if (hint) {
        ImGui::Text("%s", hint);
    }
    
    if (ImGui::InputText(label, buf, sizeof(buf))) {
        path = buf;
        changed = true;
    }
    
    return changed;
}

std::string AnalysisConfigWidget::get_cpp_standard_string(int standard) const {
    const char* standards[] = { "c++03", "c++11", "c++14", "c++17", "c++20" };
    if (standard >= 0 && standard < 5) {
        return standards[standard];
    }
    return "c++20";
}

std::string AnalysisConfigWidget::get_platform_string(int platform) const {
    const char* platforms[] = { "unix32", "unix64", "win32A", "win64" };
    if (platform >= 0 && platform < 4) {
        return platforms[platform];
    }
    return "unix64";
}

} // namespace gran_azul::widgets