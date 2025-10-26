#include "analysis_config_widget.h"
#include <imgui.h>
#include <cstring>

using namespace gran_azul::widgets;

AnalysisConfigWidget::AnalysisConfigWidget() : wip::gui::Widget("AnalysisConfigWidget") {
    // Constructor implementation
}

void AnalysisConfigWidget::update(float delta_time) {
    // Update logic if needed
}

void AnalysisConfigWidget::draw() {
    ImGui::Text("Legacy draw method - use render() instead");
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

void AnalysisConfigWidget::render_tools_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    if (ImGui::BeginTabItem("Tools")) {
        ImGui::Text("Analysis Tool Selection");
        ImGui::Separator();
        
        // Tool availability checks
        ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "Available Tools:");
        
        ImGui::Checkbox("Enable Cppcheck", &config.enable_cppcheck);
        ImGui::SameLine();
        if (ImGui::Button("Test Cppcheck")) {
            // TODO: Test cppcheck availability
            ImGui::OpenPopup("CppcheckTest");
        }
        
        if (ImGui::BeginPopup("CppcheckTest")) {
            ImGui::Text("Testing Cppcheck availability...");
            ImGui::EndPopup();
        }
        
        ImGui::Checkbox("Enable Clang-Tidy", &config.enable_clang_tidy);
        ImGui::SameLine();
        if (ImGui::Button("Test Clang-Tidy")) {
            // TODO: Test clang-tidy availability
            ImGui::OpenPopup("ClangTidyTest");
        }
        
        if (ImGui::BeginPopup("ClangTidyTest")) {
            ImGui::Text("Testing Clang-Tidy availability...");
            ImGui::EndPopup();
        }
        
        ImGui::Separator();
        ImGui::Text("Tool Options:");
        
        // Global options
        ImGui::Text("Runtime Configuration:");
        ImGui::SliderInt("Parallel Jobs", &config.cppcheck.job_count, 1, 16);
        ImGui::Checkbox("Quiet Mode", &config.cppcheck.quiet);
        
        ImGui::EndTabItem();
    }
}

void AnalysisConfigWidget::render_tool_selection_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    render_tools_tab(config); // Delegate to render_tools_tab
}

void AnalysisConfigWidget::render_cppcheck_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    if (ImGui::BeginTabItem("Cppcheck")) {
        if (!config.enable_cppcheck) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Cppcheck is disabled. Enable it in the Tools tab.");
            ImGui::EndTabItem();
            return;
        }
        
        ImGui::Text("Cppcheck Configuration");
        ImGui::Separator();
        
        // Basic Analysis Options
        if (ImGui::CollapsingHeader("Analysis Options", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable All Checks", &config.cppcheck.enable_all);
            ImGui::Checkbox("Warning", &config.cppcheck.enable_warning);
            ImGui::Checkbox("Style", &config.cppcheck.enable_style);
            ImGui::Checkbox("Performance", &config.cppcheck.enable_performance);
            ImGui::Checkbox("Portability", &config.cppcheck.enable_portability);
            ImGui::Checkbox("Information", &config.cppcheck.enable_information);
            ImGui::Checkbox("Unused Function", &config.cppcheck.enable_unused_function);
            ImGui::Checkbox("Missing Include", &config.cppcheck.enable_missing_include);
        }
        
        // Advanced Options
        if (ImGui::CollapsingHeader("Advanced Options")) {
            ImGui::SliderInt("Check Level", &config.cppcheck.check_level, 0, 1);
            ImGui::SameLine();
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("0 = Normal, 1 = Exhaustive");
            }
            
            ImGui::Checkbox("Inconclusive Results", &config.cppcheck.inconclusive);
            ImGui::Checkbox("Verbose Output", &config.cppcheck.verbose);
        }
        
        // Performance Options
        if (ImGui::CollapsingHeader("Performance")) {
            ImGui::SliderInt("Job Count", &config.cppcheck.job_count, 1, 16);
            ImGui::Checkbox("Quiet Mode", &config.cppcheck.quiet);
        }
        
        // Suppressions
        if (ImGui::CollapsingHeader("Suppressions")) {
            ImGui::Checkbox("Suppress Unused Function", &config.cppcheck.suppress_unused_function);
            ImGui::Checkbox("Suppress Missing Include (System)", &config.cppcheck.suppress_missing_include_system);
            ImGui::Checkbox("Suppress Missing Include", &config.cppcheck.suppress_missing_include);
            ImGui::Checkbox("Suppress Duplicate Conditional", &config.cppcheck.suppress_duplicate_conditional);
        }
        
        // Libraries
        if (ImGui::CollapsingHeader("Libraries")) {
            ImGui::Checkbox("Use POSIX Library", &config.cppcheck.use_posix_library);
            ImGui::Checkbox("Use MISRA Addon", &config.cppcheck.use_misra_addon);
        }
        
        // Include Paths and Definitions
        if (ImGui::CollapsingHeader("Include Paths & Definitions")) {
            ImGui::Text("Include Paths:");
            for (size_t i = 0; i < config.cppcheck.include_paths.size(); ++i) {
                ImGui::PushID((int)i);
                char buffer[512];
                strncpy(buffer, config.cppcheck.include_paths[i].c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                
                if (ImGui::InputText("##include_path", buffer, sizeof(buffer))) {
                    config.cppcheck.include_paths[i] = buffer;
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove")) {
                    config.cppcheck.include_paths.erase(config.cppcheck.include_paths.begin() + i);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
            
            if (ImGui::Button("Add Include Path")) {
                config.cppcheck.include_paths.push_back("");
            }
            
            ImGui::Separator();
            ImGui::Text("Preprocessor Definitions:");
            for (size_t i = 0; i < config.cppcheck.preprocessor_definitions.size(); ++i) {
                ImGui::PushID((int)i + 1000);
                char buffer[256];
                strncpy(buffer, config.cppcheck.preprocessor_definitions[i].c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                
                if (ImGui::InputText("##preprocessor_def", buffer, sizeof(buffer))) {
                    config.cppcheck.preprocessor_definitions[i] = buffer;
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove")) {
                    config.cppcheck.preprocessor_definitions.erase(config.cppcheck.preprocessor_definitions.begin() + i);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
            
            if (ImGui::Button("Add Definition")) {
                config.cppcheck.preprocessor_definitions.push_back("");
            }
        }
        
        ImGui::EndTabItem();
    }
}

void AnalysisConfigWidget::render_clang_tidy_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    if (ImGui::BeginTabItem("Clang-Tidy")) {
        if (!config.enable_clang_tidy) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Clang-Tidy is disabled. Enable it in the Tools tab.");
            ImGui::EndTabItem();
            return;
        }
        
        ImGui::Text("Clang-Tidy Configuration");
        ImGui::Separator();
        
        // Check Categories
        if (ImGui::CollapsingHeader("Check Categories", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Bugprone Checks", &config.clang_tidy.enable_bugprone_checks);
            ImGui::Checkbox("Performance Checks", &config.clang_tidy.enable_performance_checks);
            ImGui::Checkbox("Modernize Checks", &config.clang_tidy.enable_modernize_checks);
            ImGui::Checkbox("Readability Checks", &config.clang_tidy.enable_readability_checks);
            ImGui::Checkbox("C++ Core Guidelines", &config.clang_tidy.enable_cppcoreguidelines_checks);
            ImGui::Checkbox("Misc Checks", &config.clang_tidy.enable_misc_checks);
            ImGui::Checkbox("CERT Checks", &config.clang_tidy.enable_cert_checks);
        }
        
        // Specific Disables
        if (ImGui::CollapsingHeader("Specific Check Disables")) {
            ImGui::Checkbox("Disable Magic Numbers Check", &config.clang_tidy.disable_magic_numbers);
            ImGui::Checkbox("Disable Uppercase Literal Suffix", &config.clang_tidy.disable_uppercase_literal_suffix);
        }
        
        // Output Options
        if (ImGui::CollapsingHeader("Output Options")) {
            ImGui::Checkbox("Use Color in Output", &config.clang_tidy.use_color);
            ImGui::Checkbox("Export Fixes", &config.clang_tidy.export_fixes);
            
            ImGui::Text("Format Style:");
            char format_style[64];
            strncpy(format_style, config.clang_tidy.format_style.c_str(), sizeof(format_style) - 1);
            format_style[sizeof(format_style) - 1] = '\0';
            if (ImGui::InputText("##format_style", format_style, sizeof(format_style))) {
                config.clang_tidy.format_style = format_style;
            }
            ImGui::SameLine();
            if (ImGui::BeginCombo("##format_preset", "Presets")) {
                if (ImGui::Selectable("file")) config.clang_tidy.format_style = "file";
                if (ImGui::Selectable("llvm")) config.clang_tidy.format_style = "llvm";
                if (ImGui::Selectable("google")) config.clang_tidy.format_style = "google";
                if (ImGui::Selectable("webkit")) config.clang_tidy.format_style = "webkit";
                if (ImGui::Selectable("microsoft")) config.clang_tidy.format_style = "microsoft";
                ImGui::EndCombo();
            }
        }
        
        // Header Filter
        if (ImGui::CollapsingHeader("Header Filter")) {
            ImGui::Checkbox("Enable Header Filter Regex", &config.clang_tidy.header_filter_regex_enabled);
            if (config.clang_tidy.header_filter_regex_enabled) {
                char header_filter[256];
                strncpy(header_filter, config.clang_tidy.header_filter_regex.c_str(), sizeof(header_filter) - 1);
                header_filter[sizeof(header_filter) - 1] = '\0';
                if (ImGui::InputText("Header Filter Regex", header_filter, sizeof(header_filter))) {
                    config.clang_tidy.header_filter_regex = header_filter;
                }
            }
            ImGui::Checkbox("Include System Headers", &config.clang_tidy.system_headers);
        }
        
        // Fix Options
        if (ImGui::CollapsingHeader("Fix Options")) {
            ImGui::Checkbox("Fix Errors", &config.clang_tidy.fix_errors);
            ImGui::Checkbox("Fix Notes", &config.clang_tidy.fix_notes);
        }
        
        // Configuration File
        if (ImGui::CollapsingHeader("Configuration File")) {
            char config_file[512];
            strncpy(config_file, config.clang_tidy.config_file.c_str(), sizeof(config_file) - 1);
            config_file[sizeof(config_file) - 1] = '\0';
            if (ImGui::InputText("Config File Path", config_file, sizeof(config_file))) {
                config.clang_tidy.config_file = config_file;
            }
            ImGui::SameLine();
            if (ImGui::Button("Browse")) {
                // TODO: Open file browser
            }
        }
        
        // Custom Checks
        if (ImGui::CollapsingHeader("Custom Checks")) {
            ImGui::Text("Additional Checks:");
            for (size_t i = 0; i < config.clang_tidy.additional_checks.size(); ++i) {
                ImGui::PushID((int)i + 2000);
                char buffer[256];
                strncpy(buffer, config.clang_tidy.additional_checks[i].c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                
                if (ImGui::InputText("##additional_check", buffer, sizeof(buffer))) {
                    config.clang_tidy.additional_checks[i] = buffer;
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove")) {
                    config.clang_tidy.additional_checks.erase(config.clang_tidy.additional_checks.begin() + i);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
            
            if (ImGui::Button("Add Check")) {
                config.clang_tidy.additional_checks.push_back("");
            }
            
            ImGui::Separator();
            ImGui::Text("Disabled Checks:");
            for (size_t i = 0; i < config.clang_tidy.disabled_checks.size(); ++i) {
                ImGui::PushID((int)i + 3000);
                char buffer[256];
                strncpy(buffer, config.clang_tidy.disabled_checks[i].c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                
                if (ImGui::InputText("##disabled_check", buffer, sizeof(buffer))) {
                    config.clang_tidy.disabled_checks[i] = buffer;
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove")) {
                    config.clang_tidy.disabled_checks.erase(config.clang_tidy.disabled_checks.begin() + i);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
            
            if (ImGui::Button("Add Disabled Check")) {
                config.clang_tidy.disabled_checks.push_back("");
            }
        }
        
        ImGui::EndTabItem();
    }
}

void AnalysisConfigWidget::render_common_tab(gran_azul::ProjectConfig::AnalysisConfig& config) {
    if (ImGui::BeginTabItem("Common")) {
        ImGui::Text("Common Analysis Settings");
        ImGui::Separator();
        
        // Source Configuration
        if (ImGui::CollapsingHeader("Source Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Source path
            char source_path[512];
            strncpy(source_path, config.source_path.c_str(), sizeof(source_path) - 1);
            source_path[sizeof(source_path) - 1] = '\0';
            if (ImGui::InputText("Source Path", source_path, sizeof(source_path))) {
                config.source_path = source_path;
            }
            ImGui::SameLine();
            if (ImGui::Button("Browse##source")) {
                // TODO: Open directory browser
            }
            
            // Output file
            char output_file[512];
            strncpy(output_file, config.output_file.c_str(), sizeof(output_file) - 1);
            output_file[sizeof(output_file) - 1] = '\0';
            if (ImGui::InputText("Output File", output_file, sizeof(output_file))) {
                config.output_file = output_file;
            }
            ImGui::SameLine();
            if (ImGui::Button("Browse##output")) {
                // TODO: Open file browser
            }
            
            // Build directory
            char build_dir[512];
            strncpy(build_dir, config.build_dir.c_str(), sizeof(build_dir) - 1);
            build_dir[sizeof(build_dir) - 1] = '\0';
            if (ImGui::InputText("Build Directory", build_dir, sizeof(build_dir))) {
                config.build_dir = build_dir;
            }
            ImGui::SameLine();
            if (ImGui::Button("Browse##build")) {
                // TODO: Open directory browser
            }
        }
        
        // Standards and Platform
        if (ImGui::CollapsingHeader("Standards and Platform")) {
            const char* cpp_standards[] = {"C++03", "C++11", "C++14", "C++17", "C++20", "C++23"};
            if (ImGui::Combo("C++ Standard", &config.cpp_standard, cpp_standards, IM_ARRAYSIZE(cpp_standards))) {
                // Standard updated
            }
            
            const char* platforms[] = {"Unix32", "Unix64", "Win32A", "Win64"};
            if (ImGui::Combo("Platform", &config.platform, platforms, IM_ARRAYSIZE(platforms))) {
                // Platform updated
            }
        }
        
        ImGui::EndTabItem();
    }
}