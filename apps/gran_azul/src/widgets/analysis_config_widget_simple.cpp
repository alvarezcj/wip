#include "analysis_config_widget.h"
#include <imgui.h>
#include <cstring>

using namespace gran_azul::widgets;

AnalysisConfigWidget::AnalysisConfigWidget() : wip::gui::Widget("AnalysisConfigWidget") {
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
    ImGui::Text("Legacy draw method - use render() instead");
}

void AnalysisConfigWidget::render_tools_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    if (ImGui::BeginTabItem("Tools")) {
        ImGui::Text("Tool Selection");
        ImGui::Checkbox("Enable Cppcheck", &config.enable_cppcheck);
        ImGui::Checkbox("Enable Clang-Tidy", &config.enable_clang_tidy);
        ImGui::EndTabItem();
    }
}

void AnalysisConfigWidget::render_tool_selection_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    render_tools_tab(config); // Delegate to render_tools_tab
}

void AnalysisConfigWidget::render_cppcheck_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    if (ImGui::BeginTabItem("Cppcheck")) {
        ImGui::Text("Cppcheck Configuration");
        ImGui::Checkbox("Enable All", &config.cppcheck.enable_all);
        ImGui::Checkbox("Enable Warning", &config.cppcheck.enable_warning);
        ImGui::Checkbox("Enable Style", &config.cppcheck.enable_style);
        ImGui::Checkbox("Enable Performance", &config.cppcheck.enable_performance);
        ImGui::Checkbox("Enable Portability", &config.cppcheck.enable_portability);
        ImGui::Checkbox("Enable Information", &config.cppcheck.enable_information);
        ImGui::EndTabItem();
    }
}

void AnalysisConfigWidget::render_clang_tidy_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    if (ImGui::BeginTabItem("Clang-Tidy")) {
        ImGui::Text("Clang-Tidy Configuration");
        ImGui::Checkbox("Enable Bugprone Checks", &config.clang_tidy.enable_bugprone_checks);
        ImGui::Checkbox("Enable Performance Checks", &config.clang_tidy.enable_performance_checks);
        ImGui::Checkbox("Enable Modernize Checks", &config.clang_tidy.enable_modernize_checks);
        ImGui::Checkbox("Enable Readability Checks", &config.clang_tidy.enable_readability_checks);
        ImGui::EndTabItem();
    }
}

void AnalysisConfigWidget::render_common_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    if (ImGui::BeginTabItem("Common")) {
        ImGui::Text("Common Settings");
        
        // Source path
        static char source_path[512];
        strncpy(source_path, config.source_path.c_str(), sizeof(source_path) - 1);
        source_path[sizeof(source_path) - 1] = '\0';
        if (ImGui::InputText("Source Path", source_path, sizeof(source_path))) {
            config.source_path = source_path;
        }
        
        // Output file
        static char output_file[512];
        strncpy(output_file, config.output_file.c_str(), sizeof(output_file) - 1);
        output_file[sizeof(output_file) - 1] = '\0';
        if (ImGui::InputText("Output File", output_file, sizeof(output_file))) {
            config.output_file = output_file;
        }
        
        // Build directory
        static char build_dir[512];
        strncpy(build_dir, config.build_dir.c_str(), sizeof(build_dir) - 1);
        build_dir[sizeof(build_dir) - 1] = '\0';
        if (ImGui::InputText("Build Directory", build_dir, sizeof(build_dir))) {
            config.build_dir = build_dir;
        }
        
        // C++ Standard
        ImGui::SliderInt("C++ Standard", &config.cpp_standard, 11, 23);
        
        // Platform
        ImGui::SliderInt("Platform", &config.platform, 0, 2);
        
        ImGui::EndTabItem();
    }
}