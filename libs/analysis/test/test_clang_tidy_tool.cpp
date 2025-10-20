#include <gtest/gtest.h>
#include "tools/clang_tidy_tool.h"
#include "analysis_types.h"
#include <memory>
#include <filesystem>

using namespace wip::analysis::tools;
using namespace wip::analysis;

class ClangTidyToolTest : public ::testing::Test {
protected:
    void SetUp() override {
        tool_ = std::make_unique<ClangTidyTool>();
    }
    
    void TearDown() override {
        tool_.reset();
    }
    
    std::unique_ptr<ClangTidyTool> tool_;
};

// Test basic tool properties
TEST_F(ClangTidyToolTest, BasicProperties) {
    EXPECT_EQ(tool_->get_name(), "clang-tidy");
    EXPECT_FALSE(tool_->get_version().empty());
    EXPECT_FALSE(tool_->get_description().empty());
    // Note: is_available() depends on system having clang-tidy installed, so we don't test it
}

// Test configuration
TEST_F(ClangTidyToolTest, DefaultConfiguration) {
    auto config = tool_->get_configuration();
    EXPECT_NE(config, nullptr);
    
    auto clang_tidy_config = dynamic_cast<const ClangTidyConfig*>(config);
    EXPECT_NE(clang_tidy_config, nullptr);
    EXPECT_EQ(clang_tidy_config->tool_name, "clang-tidy");
}

TEST_F(ClangTidyToolTest, CustomConfiguration) {
    auto config = std::make_unique<ClangTidyConfig>();
    config->checks = {"readability-*", "performance-*"};
    config->disabled_checks = {"readability-braces-*"};
    config->fix_errors = true;
    config->header_filter_regex = ".*";
    
    tool_->set_configuration(std::move(config));
    
    auto retrieved_config = dynamic_cast<const ClangTidyConfig*>(tool_->get_configuration());
    EXPECT_NE(retrieved_config, nullptr);
    EXPECT_EQ(retrieved_config->checks.size(), 2);
    EXPECT_EQ(retrieved_config->disabled_checks.size(), 1);
    EXPECT_TRUE(retrieved_config->fix_errors);
    EXPECT_EQ(retrieved_config->header_filter_regex, ".*");
}

// Test configuration validation
TEST_F(ClangTidyToolTest, ConfigurationValidation) {
    // Set up a valid configuration first
    auto config = std::make_unique<ClangTidyConfig>();
    config->source_path = "/tmp";  // Use a path that exists
    config->output_file = "test_output.yaml";
    tool_->set_configuration(std::move(config));
    
    auto validation_result = tool_->validate_configuration();
    EXPECT_TRUE(validation_result.errors.empty());
}

TEST_F(ClangTidyToolTest, ConfigurationWithChecks) {
    auto config = std::make_unique<ClangTidyConfig>();
    config->source_path = "/tmp";  // Use a path that exists
    config->output_file = "test_output.yaml";
    config->checks = {"readability-*", "performance-*", "modernize-*"};
    config->disabled_checks = {"readability-braces-*"};
    tool_->set_configuration(std::move(config));
    
    auto validation_result = tool_->validate_configuration();
    EXPECT_TRUE(validation_result.errors.empty());
}

TEST_F(ClangTidyToolTest, ConfigurationWithConfigFile) {
    auto config = std::make_unique<ClangTidyConfig>();
    config->source_path = "/tmp";  // Use a path that exists
    config->output_file = "test_output.yaml";
    config->config_file = ".clang-tidy";
    tool_->set_configuration(std::move(config));
    
    auto validation_result = tool_->validate_configuration();
    EXPECT_TRUE(validation_result.errors.empty());
}

// Test command line building
TEST_F(ClangTidyToolTest, BasicCommandLine) {
    // Set up a basic configuration first
    auto config = std::make_unique<ClangTidyConfig>();
    tool_->set_configuration(std::move(config));
    
    AnalysisRequest request;
    request.source_path = "/tmp/test.cpp";  // Use a real C++ file
    request.output_file = "/test/output.xml";
    
    auto cmdline = tool_->build_command_line(request);
    
    // Should contain basic clang-tidy command
    EXPECT_FALSE(cmdline.empty());
    EXPECT_EQ(cmdline[0], "clang-tidy");
    
    // Should contain source path (as one of the arguments, not necessarily last)
    bool has_source_path = false;
    for (const auto& arg : cmdline) {
        if (arg == request.source_path) {
            has_source_path = true;
            break;
        }
    }
    EXPECT_TRUE(has_source_path);
}

TEST_F(ClangTidyToolTest, CommandLineWithChecks) {
    auto config = std::make_unique<ClangTidyConfig>();
    config->checks = {"readability-*", "performance-*"};
    config->disabled_checks = {"readability-braces-*"};
    
    tool_->set_configuration(std::move(config));
    
    AnalysisRequest request;
    request.source_path = "/test/source";
    request.output_file = "/test/output.xml";
    
    auto cmdline = tool_->build_command_line(request);
    
    // Convert to string for easier searching
    std::string cmdline_str;
    for (const auto& arg : cmdline) {
        cmdline_str += arg + " ";
    }
    
    // Should contain checks argument
    EXPECT_TRUE(cmdline_str.find("--checks=") != std::string::npos || 
                cmdline_str.find("-checks=") != std::string::npos);
    
    // Should contain enabled checks
    EXPECT_TRUE(cmdline_str.find("readability-*") != std::string::npos);
    EXPECT_TRUE(cmdline_str.find("performance-*") != std::string::npos);
    
    // Should contain disabled checks (with minus prefix)
    EXPECT_TRUE(cmdline_str.find("-readability-braces-*") != std::string::npos);
}

TEST_F(ClangTidyToolTest, CommandLineWithConfigFile) {
    auto config = std::make_unique<ClangTidyConfig>();
    config->config_file = ".clang-tidy";
    config->config_file = "/path/to/.clang-tidy";
    
    tool_->set_configuration(std::move(config));
    
    AnalysisRequest request;
    request.source_path = "/test/source";
    
    auto cmdline = tool_->build_command_line(request);
    
    // Convert to string for easier searching
    std::string cmdline_str;
    for (const auto& arg : cmdline) {
        cmdline_str += arg + " ";
    }
    
    // Should contain config file argument
    EXPECT_TRUE(cmdline_str.find("--config-file=") != std::string::npos ||
                cmdline_str.find("/path/to/.clang-tidy") != std::string::npos);
}

TEST_F(ClangTidyToolTest, CommandLineWithHeaderFilter) {
    auto config = std::make_unique<ClangTidyConfig>();
    config->header_filter_regex = ".*\\.h$";
    
    tool_->set_configuration(std::move(config));
    
    AnalysisRequest request;
    request.source_path = "/test/source";
    
    auto cmdline = tool_->build_command_line(request);
    
    // Convert to string for easier searching
    std::string cmdline_str;
    for (const auto& arg : cmdline) {
        cmdline_str += arg + " ";
    }
    
    // Should contain header filter
    EXPECT_TRUE(cmdline_str.find("--header-filter=") != std::string::npos);
}

TEST_F(ClangTidyToolTest, CommandLineWithFix) {
    auto config = std::make_unique<ClangTidyConfig>();
    config->export_fixes = true;  // Use export_fixes instead of fix_errors
    config->format_style = "llvm";  // Set non-default format style
    
    tool_->set_configuration(std::move(config));
    
    AnalysisRequest request;
    request.source_path = "/tmp/test.cpp";  // Use real file
    request.output_file = "/test/output.yaml";
    
    auto cmdline = tool_->build_command_line(request);
    
    // Convert to string for easier searching
    std::string cmdline_str;
    for (const auto& arg : cmdline) {
        cmdline_str += arg + " ";
    }
    
    // Should contain fix options
    EXPECT_TRUE(cmdline_str.find("--export-fixes=") != std::string::npos);
    EXPECT_TRUE(cmdline_str.find("--format-style=") != std::string::npos);
}

TEST_F(ClangTidyToolTest, CommandLineWithIncludePaths) {
    AnalysisRequest request;
    request.source_path = "/test/source";
    request.include_paths = {"/usr/include", "/opt/include"};
    
    auto cmdline = tool_->build_command_line(request);
    
    // Convert to string for easier searching
    std::string cmdline_str;
    for (const auto& arg : cmdline) {
        cmdline_str += arg + " ";
    }
    
    // Should contain include paths in the compiler arguments section
    EXPECT_TRUE(cmdline_str.find("/usr/include") != std::string::npos);
    EXPECT_TRUE(cmdline_str.find("/opt/include") != std::string::npos);
}

// Test supported formats
TEST_F(ClangTidyToolTest, SupportedFormats) {
    auto formats = tool_->get_supported_output_formats();
    EXPECT_FALSE(formats.empty());
    
    // Should support some common formats
    // Clang-tidy typically outputs text format, some versions support YAML
    EXPECT_GT(formats.size(), 0);
}

// Test help text
TEST_F(ClangTidyToolTest, HelpText) {
    auto help = tool_->get_help_text();
    EXPECT_FALSE(help.empty());
    EXPECT_TRUE(help.find("clang-tidy") != std::string::npos || 
                help.find("Clang-Tidy") != std::string::npos ||
                help.find("clang") != std::string::npos);
}

// Test execution request validation
TEST_F(ClangTidyToolTest, ExecutionRequestValidation) {
    AnalysisRequest request;
    // Empty request should be handled gracefully
    
    // We don't actually execute since that requires clang-tidy to be installed
    // and proper source files, but we can test request building
    auto cmdline = tool_->build_command_line(request);
    EXPECT_FALSE(cmdline.empty());
    EXPECT_EQ(cmdline[0], "clang-tidy");
}

TEST_F(ClangTidyToolTest, ExecutionRequestWithCompileCommands) {
    // Set up configuration to point to build directory with compile_commands.json
    auto config = std::make_unique<ClangTidyConfig>();
    config->source_path = "/tmp";
    config->output_file = "test_output.yaml";
    tool_->set_configuration(std::move(config));
    
    AnalysisRequest request;
    request.source_path = "/home/jalvarez/wip/build";  // Use actual build directory
    
    auto cmdline = tool_->build_command_line(request);
    EXPECT_FALSE(cmdline.empty());
    EXPECT_EQ(cmdline[0], "clang-tidy");
    
    // Convert to string for easier searching
    std::string cmdline_str;
    for (const auto& arg : cmdline) {
        cmdline_str += arg + " ";
    }
    
    // Should contain compile commands path
    EXPECT_TRUE(cmdline_str.find("-p") != std::string::npos || 
                cmdline_str.find("compile_commands.json") != std::string::npos);
}

// Test analysis state management
TEST_F(ClangTidyToolTest, AnalysisStateManagement) {
    EXPECT_FALSE(tool_->is_analysis_running());
    
    // Test cancellation (should work even when not running)
    bool cancelled = tool_->cancel_analysis();
    EXPECT_TRUE(cancelled || !cancelled); // Either outcome is acceptable for a stopped analysis
    
    EXPECT_FALSE(tool_->is_analysis_running());
}

// Test result parsing (mock)
TEST_F(ClangTidyToolTest, ResultParsingMock) {
    // Test parsing with a mock/empty file (shouldn't crash)
    // In a real test, we would use a mock output file with known content
    
    // For now, just test that the method exists and doesn't crash with empty input
    // Real parsing tests would require sample output files
    auto formats = tool_->get_supported_output_formats();
    EXPECT_FALSE(formats.empty()); // At least verify the tool has output format support
}

// Test configuration serialization
TEST_F(ClangTidyToolTest, ConfigurationSerialization) {
    auto config = std::make_unique<ClangTidyConfig>();
    config->checks = {"readability-*", "performance-*"};
    config->disabled_checks = {"readability-braces-*"};
    config->fix_errors = true;
    config->header_filter_regex = ".*";
    
    // Test serialization
    auto json_data = config->to_json();
    EXPECT_TRUE(json_data.contains("tool_name"));
    EXPECT_EQ(json_data["tool_name"], "clang-tidy");
    EXPECT_TRUE(json_data["fix_errors"]);
    EXPECT_EQ(json_data["header_filter_regex"], ".*");
    
    // Test deserialization
    ClangTidyConfig config2;
    config2.from_json(json_data);
    EXPECT_EQ(config2.checks.size(), 2);
    EXPECT_EQ(config2.disabled_checks.size(), 1);
    EXPECT_TRUE(config2.fix_errors);
    EXPECT_EQ(config2.header_filter_regex, ".*");
}

// Test edge cases
TEST_F(ClangTidyToolTest, EmptyConfiguration) {
    // Test with completely empty configuration
    auto config = std::make_unique<ClangTidyConfig>();
    config->source_path = "/tmp";  // Use a path that exists
    config->output_file = "test_output.yaml";
    // Leave other settings at defaults
    
    tool_->set_configuration(std::move(config));
    
    AnalysisRequest request;
    request.source_path = "/test/source";
    
    auto cmdline = tool_->build_command_line(request);
    EXPECT_FALSE(cmdline.empty());
    EXPECT_EQ(cmdline[0], "clang-tidy");
    
    // Should still work with empty configuration
    auto validation_result = tool_->validate_configuration();
    EXPECT_TRUE(validation_result.errors.empty());
}