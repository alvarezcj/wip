#include "cppcheck_config_widget.h"
#include "path_selector_widget.h"
#include <imgui.h>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace gran_azul::widgets {

CppcheckConfigWidget::CppcheckConfigWidget() {
    // Initialize with defaults
    
    // Create path selectors
    source_path_selector_ = std::make_unique<PathSelectorWidget>("Source Path:", PathType::Folder, 120.0f);
    build_path_selector_ = std::make_unique<PathSelectorWidget>("Build Directory:", PathType::Folder, 120.0f);
    
    // Set up callbacks
    source_path_selector_->set_callback([this](const std::string& path) {
        std::string relative_path = convert_to_relative_path(path);
        strncpy(config_.source_path, relative_path.c_str(), sizeof(config_.source_path) - 1);
        config_.source_path[sizeof(config_.source_path) - 1] = '\0';
    });
    
    build_path_selector_->set_callback([this](const std::string& path) {
        std::string relative_path = convert_to_relative_path(path);
        strncpy(config_.build_dir, relative_path.c_str(), sizeof(config_.build_dir) - 1);
        config_.build_dir[sizeof(config_.build_dir) - 1] = '\0';
    });
}

CppcheckConfigWidget::~CppcheckConfigWidget() = default;

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
        if (!has_project_) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No project loaded. Please create or open a project.");
            return;
        }
        
        // Update path selectors with current config values
        source_path_selector_->set_path(config_.source_path);
        build_path_selector_->set_path(config_.build_dir);
        
        // Draw path selectors
        source_path_selector_->draw_inline();
        
        ImGui::Spacing();
        
        // Build directory is now automatically managed at .azul-cache/cppcheck-build
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Build Directory: Automatically managed at .azul-cache/cppcheck-build");
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Note: Output files will be saved in the project directory");
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
    
    // Disable analysis button if no project is loaded
    ImGui::BeginDisabled(!has_project_);
    if (ImGui::Button("Run Analysis", ImVec2(120, 0))) {
        if (on_run_analysis_) {
            on_run_analysis_(config_);
        }
    }
    ImGui::EndDisabled();
    
    ImGui::SameLine();
    if (ImGui::Button("Test cppcheck --version", ImVec2(160, 0))) {
        if (on_run_version_) {
            on_run_version_();
        }
    }
    ImGui::SameLine();
    
    ImGui::BeginDisabled(!has_project_);
    if (ImGui::Button("Reset to Defaults", ImVec2(140, 0))) {
        config_ = CppcheckConfig(); // Reset to defaults
    }
    ImGui::EndDisabled();
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

std::string CppcheckConfigWidget::convert_to_relative_path(const std::string& absolute_path) {
    if (project_base_path_.empty() || absolute_path.empty()) {
        return absolute_path;
    }
    
    try {
        std::filesystem::path abs_path = std::filesystem::absolute(absolute_path);
        std::filesystem::path base_path = std::filesystem::absolute(project_base_path_);
        
        // Check if the path is under the project base
        std::filesystem::path relative = std::filesystem::relative(abs_path, base_path);
        
        // If relative path doesn't start with "../../../..", it's within project
        std::string relative_str = relative.string();
        if (relative_str.find("..") != 0) {
            return relative_str;
        } else {
            // Path is outside project, return absolute
            return absolute_path;
        }
    } catch (const std::exception& e) {
        std::cout << "[CPPCHECK_WIDGET] Error calculating relative path: " << e.what() << std::endl;
        return absolute_path;
    }
}

std::string CppcheckConfigWidget::get_project_base_path() {
    return project_base_path_;
}

} // namespace gran_azul::widgets