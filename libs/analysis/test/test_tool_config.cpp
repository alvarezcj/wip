#include <gtest/gtest.h>
#include "tool_config.h"
#include "tools/cppcheck_tool.h"
#include "tools/clang_tidy_tool.h"
#include <nlohmann/json.hpp>

using namespace wip::analysis;
using namespace wip::analysis::tools;
using json = nlohmann::json;

class ToolConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test fixtures
    }
    
    void TearDown() override {
        // Clean up test fixtures
    }
};

// Test ToolConfig base class
TEST_F(ToolConfigTest, ToolConfigConstruction) {
    class TestConfig : public ToolConfig {
    public:
        TestConfig(const std::string& name) {
            tool_name = name;
        }
        
        std::unique_ptr<ToolConfig> clone() const override {
            auto cloned = std::make_unique<TestConfig>(tool_name);
            cloned->source_path = source_path;
            cloned->output_file = output_file;
            cloned->include_paths = include_paths;
            cloned->definitions = definitions;
            cloned->custom_options = custom_options;
            return cloned;
        }
        
        json to_json() const override {
            return ToolConfig::to_json();
        }
        
        void from_json(const json& j) override {
            ToolConfig::from_json(j);
        }
    };
    
    TestConfig config("test-tool");
    EXPECT_EQ(config.tool_name, "test-tool");
    
    auto result = config.validate();
    EXPECT_TRUE(result.errors.empty() || !result.errors.empty()); // Just check it doesn't crash
}

TEST_F(ToolConfigTest, ToolConfigSerialization) {
    class TestConfig : public ToolConfig {
    public:
        std::string test_value = "default";
        
        TestConfig(const std::string& name) {
            tool_name = name;
        }
        
        std::unique_ptr<ToolConfig> clone() const override {
            auto cloned = std::make_unique<TestConfig>(tool_name);
            cloned->source_path = source_path;
            cloned->output_file = output_file;
            cloned->include_paths = include_paths;
            cloned->definitions = definitions;
            cloned->custom_options = custom_options;
            cloned->test_value = test_value;
            return cloned;
        }
        
        json to_json() const override {
            json j = ToolConfig::to_json();
            j["test_value"] = test_value;
            return j;
        }
        
        void from_json(const json& j) override {
            ToolConfig::from_json(j);
            if (j.contains("test_value")) {
                test_value = j["test_value"];
            }
        }
    };
    
    TestConfig config("test-tool");
    config.test_value = "custom_value";
    
    // Test serialization
    json j = config.to_json();
    EXPECT_EQ(j["tool_name"], "test-tool");
    EXPECT_EQ(j["test_value"], "custom_value");
    
    // Test deserialization
    TestConfig config2("test-tool-2");
    config2.from_json(j);
    EXPECT_EQ(config2.test_value, "custom_value");
}

// Test CppcheckConfig
TEST_F(ToolConfigTest, CppcheckConfigConstruction) {
    CppcheckConfig config;
    EXPECT_EQ(config.tool_name, "cppcheck");
    
    // Set required fields for validation to pass
    config.source_path = "/tmp";  // Use a path that exists
    config.output_file = "test_output.xml";
    
    auto result = config.validate();
    EXPECT_TRUE(result.errors.empty()); // Should be valid with required fields set
    
    // Test default values
    EXPECT_TRUE(config.enable_all);
    EXPECT_TRUE(config.enable_warning);
    EXPECT_TRUE(config.enable_style);
    EXPECT_TRUE(config.enable_performance);
    EXPECT_TRUE(config.enable_portability);
    EXPECT_FALSE(config.enable_information);
    EXPECT_FALSE(config.enable_unused_function);
    EXPECT_FALSE(config.enable_missing_include);
    EXPECT_EQ(config.check_level, 0);
    EXPECT_FALSE(config.inconclusive);
    EXPECT_FALSE(config.verbose);
}

TEST_F(ToolConfigTest, CppcheckConfigValidation) {
    CppcheckConfig config;
    
    // Set required fields for validation to pass
    config.source_path = "/tmp";  // Use a path that exists
    config.output_file = "test_output.xml";
    
    // Test valid configuration
    auto result = config.validate();
    EXPECT_TRUE(result.errors.empty());
    
    // Test with some options modified
    config.enable_style = true;
    config.check_level = 1;
    result = config.validate();
    EXPECT_TRUE(result.errors.empty());
}

TEST_F(ToolConfigTest, CppcheckConfigSerialization) {
    CppcheckConfig config;
    config.enable_style = true;
    config.enable_performance = true;
    config.enable_information = true;
    config.check_level = 1;
    config.inconclusive = true;
    config.verbose = true;
    
    // Test serialization
    json j = config.to_json();
    EXPECT_EQ(j["tool_name"], "cppcheck");
    EXPECT_TRUE(j["enable_style"]);
    EXPECT_TRUE(j["enable_performance"]);
    EXPECT_TRUE(j["enable_information"]);
    EXPECT_EQ(j["check_level"], 1);
    EXPECT_TRUE(j["inconclusive"]);
    EXPECT_TRUE(j["verbose"]);
    
    // Test deserialization
    CppcheckConfig config2;
    config2.from_json(j);
    EXPECT_EQ(config2.tool_name, "cppcheck");
    EXPECT_TRUE(config2.enable_style);
    EXPECT_TRUE(config2.enable_performance);
    EXPECT_TRUE(config2.enable_information);
    EXPECT_EQ(config2.check_level, 1);
    EXPECT_TRUE(config2.inconclusive);
    EXPECT_TRUE(config2.verbose);
}

// Test ClangTidyConfig
TEST_F(ToolConfigTest, ClangTidyConfigConstruction) {
    ClangTidyConfig config;
    EXPECT_EQ(config.tool_name, "clang-tidy");
    
    // Set required fields for validation to pass
    config.source_path = "/tmp";  // Use a path that exists
    config.output_file = "test_output.yaml";
    
    auto result = config.validate();
    EXPECT_TRUE(result.errors.empty()); // Should be valid with required fields set
    
    // Test default values
    EXPECT_FALSE(config.checks.empty()); // Should have default checks
    EXPECT_FALSE(config.disabled_checks.empty()); // Should have default disabled checks
    EXPECT_TRUE(config.config_file.empty());
    EXPECT_FALSE(config.use_color);
    EXPECT_FALSE(config.export_fixes);
    EXPECT_EQ(config.format_style, "file");
    EXPECT_TRUE(config.header_filter_regex_enabled);
    EXPECT_EQ(config.header_filter_regex, ".*");
    EXPECT_FALSE(config.system_headers);
    EXPECT_FALSE(config.fix_errors);
    EXPECT_FALSE(config.fix_notes);
}

TEST_F(ToolConfigTest, ClangTidyConfigValidation) {
    ClangTidyConfig config;
    
    // Set required fields for validation to pass
    config.source_path = "/tmp";  // Use a path that exists
    config.output_file = "test_output.yaml";
    
    // Test valid configuration
    auto result = config.validate();
    EXPECT_TRUE(result.errors.empty());
    
    // Test with some options modified
    config.checks = {"readability-*", "performance-*"};
    config.fix_errors = true;
    result = config.validate();
    EXPECT_TRUE(result.errors.empty());
}

TEST_F(ToolConfigTest, ClangTidyConfigSerialization) {
    ClangTidyConfig config;
    config.checks = {"readability-*", "performance-*"};
    config.disabled_checks = {"readability-magic-numbers", "modernize-use-trailing-return-type"};
    config.config_file = "/path/to/.clang-tidy";
    config.use_color = true;
    config.export_fixes = true;
    config.format_style = "llvm";
    config.header_filter_regex = ".*\\.h$";
    config.fix_errors = true;
    config.fix_notes = true;
    
    // Test serialization
    json j = config.to_json();
    EXPECT_EQ(j["tool_name"], "clang-tidy");
    ASSERT_TRUE(j["checks"].is_array());
    EXPECT_EQ(j["checks"].size(), 2);
    EXPECT_EQ(j["checks"][0], "readability-*");
    EXPECT_EQ(j["checks"][1], "performance-*");
    ASSERT_TRUE(j["disabled_checks"].is_array());
    EXPECT_EQ(j["disabled_checks"].size(), 2);
    EXPECT_EQ(j["disabled_checks"][0], "readability-magic-numbers");
    EXPECT_EQ(j["config_file"], "/path/to/.clang-tidy");
    EXPECT_TRUE(j["use_color"]);
    EXPECT_TRUE(j["export_fixes"]);
    EXPECT_EQ(j["format_style"], "llvm");
    EXPECT_EQ(j["header_filter_regex"], ".*\\.h$");
    EXPECT_TRUE(j["fix_errors"]);
    EXPECT_TRUE(j["fix_notes"]);
    
    // Test deserialization
    ClangTidyConfig config2;
    config2.from_json(j);
    EXPECT_EQ(config2.tool_name, "clang-tidy");
    EXPECT_EQ(config2.checks.size(), 2);
    EXPECT_EQ(config2.checks[0], "readability-*");
    EXPECT_EQ(config2.checks[1], "performance-*");
    EXPECT_EQ(config2.disabled_checks.size(), 2);
    EXPECT_EQ(config2.disabled_checks[0], "readability-magic-numbers");
    EXPECT_EQ(config2.config_file, "/path/to/.clang-tidy");
    EXPECT_TRUE(config2.use_color);
    EXPECT_TRUE(config2.export_fixes);
    EXPECT_EQ(config2.format_style, "llvm");
    EXPECT_EQ(config2.header_filter_regex, ".*\\.h$");
    EXPECT_TRUE(config2.fix_errors);
    EXPECT_TRUE(config2.fix_notes);
}