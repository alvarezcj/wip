#include <gtest/gtest.h>
#include "analysis_types.h"
#include <chrono>
#include <string>

using namespace wip::analysis;

class AnalysisTypesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test fixtures
    }
    
    void TearDown() override {
        // Clean up test fixtures
    }
};

// Test AnalysisRequest structure
TEST_F(AnalysisTypesTest, AnalysisRequestConstruction) {
    AnalysisRequest request;
    EXPECT_TRUE(request.source_path.empty());
    EXPECT_TRUE(request.output_file.empty());
    EXPECT_TRUE(request.include_paths.empty());
    EXPECT_TRUE(request.definitions.empty());
}

TEST_F(AnalysisTypesTest, AnalysisRequestSetValues) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    request.output_file = "output.xml";
    request.include_paths = {"/usr/include", "/opt/include"};
    
    EXPECT_EQ(request.source_path, "/test/path");
    EXPECT_EQ(request.output_file, "output.xml");
    EXPECT_EQ(request.include_paths.size(), 2);
    EXPECT_EQ(request.include_paths[0], "/usr/include");
    EXPECT_EQ(request.include_paths[1], "/opt/include");
}

// Test AnalysisResult structure
TEST_F(AnalysisTypesTest, AnalysisResultConstruction) {
    AnalysisResult result;
    EXPECT_TRUE(result.tool_name.empty());
    EXPECT_TRUE(result.analysis_id.empty());
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.error_message.empty());
    EXPECT_EQ(result.files_analyzed, 0);
    EXPECT_TRUE(result.issues.empty());
}

TEST_F(AnalysisTypesTest, AnalysisResultSetValues) {
    AnalysisResult result;
    result.tool_name = "test-tool";
    result.analysis_id = "test-123";
    result.success = true;
    result.files_analyzed = 5;
    
    EXPECT_EQ(result.tool_name, "test-tool");
    EXPECT_EQ(result.analysis_id, "test-123");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.files_analyzed, 5);
}

// Test AnalysisIssue structure
TEST_F(AnalysisTypesTest, AnalysisIssueConstruction) {
    AnalysisIssue issue;
    EXPECT_TRUE(issue.file_path.empty());
    EXPECT_EQ(issue.line_number, 0);
    EXPECT_EQ(issue.column_number, 0);
    EXPECT_EQ(issue.severity, IssueSeverity::Warning);
    EXPECT_TRUE(issue.message.empty());
    EXPECT_TRUE(issue.rule_id.empty());
    EXPECT_EQ(issue.category, IssueCategory::Style);
}

TEST_F(AnalysisTypesTest, AnalysisIssueSetValues) {
    AnalysisIssue issue;
    issue.file_path = "test.cpp";
    issue.line_number = 42;
    issue.column_number = 10;
    issue.severity = IssueSeverity::Error;
    issue.message = "Test error message";
    issue.rule_id = "test-rule";
    issue.category = IssueCategory::Bug;
    
    EXPECT_EQ(issue.file_path, "test.cpp");
    EXPECT_EQ(issue.line_number, 42);
    EXPECT_EQ(issue.column_number, 10);
    EXPECT_EQ(issue.severity, IssueSeverity::Error);
    EXPECT_EQ(issue.message, "Test error message");
    EXPECT_EQ(issue.rule_id, "test-rule");
    EXPECT_EQ(issue.category, IssueCategory::Bug);
}

// Test AnalysisProgress structure
TEST_F(AnalysisTypesTest, AnalysisProgressConstruction) {
    AnalysisProgress progress;
    EXPECT_EQ(progress.total_files, 0);
    EXPECT_EQ(progress.processed_files, 0);
    EXPECT_TRUE(progress.current_file.empty());
    EXPECT_TRUE(progress.status_message.empty());
}

TEST_F(AnalysisTypesTest, AnalysisProgressCalculateRatio) {
    AnalysisProgress progress;
    progress.total_files = 100;
    progress.processed_files = 25;
    
    EXPECT_DOUBLE_EQ(progress.get_progress_ratio(), 0.25);
    
    progress.processed_files = 0;
    EXPECT_DOUBLE_EQ(progress.get_progress_ratio(), 0.0);
    
    progress.processed_files = 100;
    EXPECT_DOUBLE_EQ(progress.get_progress_ratio(), 1.0);
}

TEST_F(AnalysisTypesTest, AnalysisProgressEdgeCases) {
    AnalysisProgress progress;
    
    // Test division by zero protection
    progress.total_files = 0;
    progress.processed_files = 0;
    EXPECT_DOUBLE_EQ(progress.get_progress_ratio(), 0.0);
    
    // Test processed > total (should be capped at 1.0)
    progress.total_files = 10;
    progress.processed_files = 15;
    EXPECT_DOUBLE_EQ(progress.get_progress_ratio(), 1.0);
}

// Test IssueSeverity enum
TEST_F(AnalysisTypesTest, IssueSeverityValues) {
    EXPECT_EQ(static_cast<int>(IssueSeverity::Info), 0);
    EXPECT_EQ(static_cast<int>(IssueSeverity::Warning), 1);
    EXPECT_EQ(static_cast<int>(IssueSeverity::Error), 2);
    EXPECT_EQ(static_cast<int>(IssueSeverity::Critical), 3);
}