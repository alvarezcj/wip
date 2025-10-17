#include "cppcheck_config_widget.h"
#include <imgui.h>
#include <iostream>
#include <sstream>

namespace gran_azul::widgets {

CppcheckConfigWidget::CppcheckConfigWidget() {
    // Initialize with defaults
}

void CppcheckConfigWidget::update([[maybe_unused]] float delta_time) {
    // No specific update logic needed for this widget
}

void CppcheckConfigWidget::draw() {
    // Header
    ImGui::Text("Configure Cppcheck Static Analysis");
    ImGui::Separator();
    
    // Render all sections
    render_source_config();
    render_analysis_options();
    render_standards_platform();
    render_performance_settings();
    render_suppressions();
    render_libraries_addons();
    render_action_buttons();
    render_command_preview();
}

void CppcheckConfigWidget::render_source_config() {
    if (ImGui::CollapsingHeader("Source Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Source Path:");
        ImGui::InputText("##source_path", config_.source_path, sizeof(config_.source_path));
        if (ImGui::Button("Select Directory")) {
            if (on_select_directory_) {
                std::string selected_dir = on_select_directory_();
                if (!selected_dir.empty()) {
                    strncpy(config_.source_path, selected_dir.c_str(), sizeof(config_.source_path) - 1);
                    config_.source_path[sizeof(config_.source_path) - 1] = '\0';
                }
            } else {
                std::cout << "[CPPCHECK_WIDGET] Directory picker not implemented yet\n";
            }
        }
        
        ImGui::Spacing();
        ImGui::Text("Output File:");
        ImGui::InputText("##output_file", config_.output_file, sizeof(config_.output_file));
        
        ImGui::Spacing();
        ImGui::Text("Build Directory (optional):");
        ImGui::InputText("##build_dir", config_.build_dir, sizeof(config_.build_dir));
        ImGui::SameLine();
        if (ImGui::Button("Create")) {
            if (on_create_directory_) {
                on_create_directory_(config_);
            }
        }
    }
}

void CppcheckConfigWidget::render_analysis_options() {
    if (ImGui::CollapsingHeader("Analysis Options", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable All Checks##enable_all", &config_.enable_all);
        
        if (!config_.enable_all) {
            ImGui::Indent();
            ImGui::Checkbox("Warnings##enable_warning", &config_.enable_warning);
            ImGui::Checkbox("Style##enable_style", &config_.enable_style);
            ImGui::Checkbox("Performance##enable_performance", &config_.enable_performance);
            ImGui::Checkbox("Portability##enable_portability", &config_.enable_portability);
            ImGui::Checkbox("Information##enable_information", &config_.enable_information);
            ImGui::Checkbox("Unused Functions##enable_unused_function", &config_.enable_unused_function);
            ImGui::Checkbox("Missing Includes##enable_missing_include", &config_.enable_missing_include);
            ImGui::Unindent();
        }
        
        ImGui::Spacing();
        ImGui::Text("Analysis Level:");
        const char* levels[] = {"Normal", "Exhaustive"};
        ImGui::Combo("##check_level", &config_.check_level, levels, IM_ARRAYSIZE(levels));
        
        ImGui::Checkbox("Include Inconclusive Results##inconclusive", &config_.inconclusive);
        ImGui::Checkbox("Verbose Output##verbose", &config_.verbose);
    }
}



void CppcheckConfigWidget::render_standards_platform() {
    if (ImGui::CollapsingHeader("Standards & Platform")) {
        const char* standards[] = {"C++03", "C++11", "C++14", "C++17", "C++20"};
        ImGui::Combo("##cpp_standard", &config_.cpp_standard, standards, IM_ARRAYSIZE(standards));
        ImGui::SameLine();
        ImGui::Text("C++ Standard");
        
        const char* platforms[] = {"Unix 32-bit", "Unix 64-bit", "Windows 32-bit", "Windows 64-bit"};
        ImGui::Combo("##platform", &config_.platform, platforms, IM_ARRAYSIZE(platforms));
        ImGui::SameLine();
        ImGui::Text("Target Platform");
    }
}

void CppcheckConfigWidget::render_performance_settings() {
    if (ImGui::CollapsingHeader("Performance Settings")) {
        ImGui::SliderInt("##job_count", &config_.job_count, 1, 16);
        ImGui::SameLine();
        ImGui::Text("Job Count (Threads)");
        ImGui::Checkbox("Quiet Mode##quiet", &config_.quiet);
    }
}

void CppcheckConfigWidget::render_suppressions() {
    if (ImGui::CollapsingHeader("Suppressions")) {
        ImGui::Checkbox("Suppress Unused Functions##suppress_unused", &config_.suppress_unused_function);
        ImGui::Checkbox("Suppress Missing System Includes##suppress_missing_sys", &config_.suppress_missing_include_system);
        ImGui::Checkbox("Suppress Missing Includes##suppress_missing", &config_.suppress_missing_include);
        ImGui::Checkbox("Suppress Duplicate Conditionals##suppress_duplicate", &config_.suppress_duplicate_conditional);
    }
}

void CppcheckConfigWidget::render_libraries_addons() {
    if (ImGui::CollapsingHeader("Libraries & Addons")) {
        ImGui::Checkbox("Use POSIX Library##use_posix", &config_.use_posix_library);
        ImGui::Checkbox("Use MISRA Addon##use_misra", &config_.use_misra_addon);
    }
}

void CppcheckConfigWidget::render_action_buttons() {
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Run Analysis", ImVec2(120, 0))) {
        if (on_run_analysis_) {
            on_run_analysis_(config_);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Test cppcheck --version", ImVec2(160, 0))) {
        if (on_run_version_) {
            on_run_version_();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset to Defaults", ImVec2(140, 0))) {
        config_ = CppcheckConfig(); // Reset to defaults
    }
}

void CppcheckConfigWidget::render_command_preview() {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Command Preview:");
    ImGui::BeginChild("CommandPreview", ImVec2(0, 100), true);
    
    std::string preview = generate_command_preview();
    ImGui::TextWrapped("%s", preview.c_str());
    ImGui::EndChild();
}

std::string CppcheckConfigWidget::generate_command_preview() const {
    std::string preview = "cppcheck";
    
    // Enable checks
    if (config_.enable_all) {
        preview += " --enable=all";
    } else {
        std::string enables = "";
        bool first = true;
        if (config_.enable_warning) {
            if (!first) enables += ",";
            enables += "warning";
            first = false;
        }
        if (config_.enable_style) {
            if (!first) enables += ",";
            enables += "style";
            first = false;
        }
        if (config_.enable_performance) {
            if (!first) enables += ",";
            enables += "performance";
            first = false;
        }
        if (config_.enable_portability) {
            if (!first) enables += ",";
            enables += "portability";
            first = false;
        }
        if (config_.enable_information) {
            if (!first) enables += ",";
            enables += "information";
            first = false;
        }
        if (config_.enable_unused_function) {
            if (!first) enables += ",";
            enables += "unusedFunction";
            first = false;
        }
        if (config_.enable_missing_include) {
            if (!first) enables += ",";
            enables += "missingInclude";
            first = false;
        }
        if (!first) {
            preview += " --enable=" + enables;
        }
    }
    
    // Analysis level and options
    if (config_.check_level == 1) {
        preview += " --check-level=exhaustive";
    }
    if (config_.inconclusive) {
        preview += " --inconclusive";
    }
    if (config_.verbose) {
        preview += " --verbose";
    }
    
    // Standards and platform
    const char* standards[] = {"c++03", "c++11", "c++14", "c++17", "c++20"};
    preview += " --std=" + std::string(standards[config_.cpp_standard]);
    
    const char* platforms[] = {"unix32", "unix64", "win32A", "win64"};
    preview += " --platform=" + std::string(platforms[config_.platform]);
    
    // Performance options
    if (config_.job_count > 1) {
        preview += " -j " + std::to_string(config_.job_count);
    }
    if (config_.quiet) {
        preview += " --quiet";
    }

    
    // Libraries and addons
    if (config_.use_posix_library) {
        preview += " --library=posix";
    }
    if (config_.use_misra_addon) {
        preview += " --addon=misra";
    }
    
    // Build directory
    if (strlen(config_.build_dir) > 0) {
        preview += " --cppcheck-build-dir=" + std::string(config_.build_dir);
    }
    
    // Suppressions
    if (config_.suppress_unused_function) {
        preview += " --suppress=unusedFunction";
    }
    if (config_.suppress_missing_include_system) {
        preview += " --suppress=missingIncludeSystem";
    }
    if (config_.suppress_missing_include) {
        preview += " --suppress=missingInclude";
    }
    if (config_.suppress_duplicate_conditional) {
        preview += " --suppress=duplicateConditionalAssign";
    }
    
    // Output format: Always use JSON for easy parsing
    preview += " --template={\"file\":\"{file}\",\"line\":{line},\"column\":{column},\"severity\":\"{severity}\",\"id\":\"{id}\",\"message\":\"{message}\",\"cwe\":{cwe}}";
    preview += " --output-file=" + std::string(config_.output_file);
    
    // Source path (last argument)
    preview += " " + std::string(config_.source_path);
    
    return preview;
}

std::vector<std::string> CppcheckConfigWidget::generate_command_args() const {
    std::vector<std::string> args;
    
    // Enable checks
    if (config_.enable_all) {
        args.push_back("--enable=all");
    } else {
        std::string enables = "--enable=";
        bool first = true;
        if (config_.enable_warning) {
            if (!first) enables += ",";
            enables += "warning";
            first = false;
        }
        if (config_.enable_style) {
            if (!first) enables += ",";
            enables += "style";
            first = false;
        }
        if (config_.enable_performance) {
            if (!first) enables += ",";
            enables += "performance";
            first = false;
        }
        if (config_.enable_portability) {
            if (!first) enables += ",";
            enables += "portability";
            first = false;
        }
        if (config_.enable_information) {
            if (!first) enables += ",";
            enables += "information";
            first = false;
        }
        if (config_.enable_unused_function) {
            if (!first) enables += ",";
            enables += "unusedFunction";
            first = false;
        }
        if (config_.enable_missing_include) {
            if (!first) enables += ",";
            enables += "missingInclude";
            first = false;
        }
        if (!first) {
            args.push_back(enables);
        }
    }
    
    // Analysis level and options
    if (config_.check_level == 1) {
        args.push_back("--check-level=exhaustive");
    }
    if (config_.inconclusive) {
        args.push_back("--inconclusive");
    }
    if (config_.verbose) {
        args.push_back("--verbose");
    }
    
    // Output format: Always use JSON for easy parsing
    args.push_back("--template={\"file\":\"{file}\",\"line\":{line},\"column\":{column},\"severity\":\"{severity}\",\"id\":\"{id}\",\"message\":\"{message}\",\"cwe\":{cwe}}");
    args.push_back("--output-file=" + std::string(config_.output_file));
    
    // Standards and platform
    const char* standards[] = {"c++03", "c++11", "c++14", "c++17", "c++20"};
    args.push_back("--std=" + std::string(standards[config_.cpp_standard]));
    
    const char* platforms[] = {"unix32", "unix64", "win32A", "win64"};
    args.push_back("--platform=" + std::string(platforms[config_.platform]));
    
    // Performance options
    if (config_.job_count > 1) {
        args.push_back("-j");
        args.push_back(std::to_string(config_.job_count));
    }
    if (config_.quiet) {
        args.push_back("--quiet");
    }

    
    // Libraries and addons
    if (config_.use_posix_library) {
        args.push_back("--library=posix");
    }
    if (config_.use_misra_addon) {
        args.push_back("--addon=misra");
    }
    
    // Build directory
    if (strlen(config_.build_dir) > 0) {
        args.push_back("--cppcheck-build-dir=" + std::string(config_.build_dir));
    }
    
    // Suppressions
    if (config_.suppress_unused_function) {
        args.push_back("--suppress=unusedFunction");
    }
    if (config_.suppress_missing_include_system) {
        args.push_back("--suppress=missingIncludeSystem");
    }
    if (config_.suppress_missing_include) {
        args.push_back("--suppress=missingInclude");
    }
    if (config_.suppress_duplicate_conditional) {
        args.push_back("--suppress=duplicateConditionalAssign");
    }
    
    // Source path (last argument)
    args.push_back(config_.source_path);
    
    return args;
}

} // namespace gran_azul::widgets