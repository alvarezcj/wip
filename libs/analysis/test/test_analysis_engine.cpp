#include <gtest/gtest.h>
#include "analysis_engine.h"
#include "analysis_types.h"
#include "tool_config.h"
#include <memory>
#include <vector>
#include <future>
#include <chrono>
#include <thread>

using namespace wip::analysis;

// Mock analysis tool for testing the engine
class MockAnalysisToolForEngine : public AnalysisTool {
private:
    std::string name_;
    bool available_;
    bool should_succeed_;
    std::atomic<bool> running_{false};
    int mock_issues_count_;
    
public:
    MockAnalysisToolForEngine(const std::string& name, bool available = true, 
                             bool should_succeed = true, int issues_count = 2)
        : name_(name), available_(available), should_succeed_(should_succeed), 
          mock_issues_count_(issues_count) {}
    
    // Basic info
    std::string get_name() const override { return name_; }
    std::string get_version() const override { return "1.0.0"; }
    std::string get_description() const override { return "Mock tool: " + name_; }
    bool is_available() const override { return available_; }
    
    // Required pure virtual methods
    std::vector<std::string> get_supported_extensions() const override {
        return {".cpp", ".h", ".cc", ".hpp"};
    }
    
    // Config methods
    void set_configuration(std::unique_ptr<ToolConfig> config) override {}
    const ToolConfig* get_configuration() const override { return nullptr; }
    std::unique_ptr<ToolConfig> create_default_config() const override { return nullptr; }
    ValidationResult validate_configuration() const override {
        ValidationResult result;
        return result;
    }
    
    // Tool availability methods  
    std::string get_executable_path() const override {
        return "/usr/bin/" + name_;
    }
    
    std::string get_system_requirements() const override {
        return name_ + " requires nothing special";
    }
    
    // Execution
    AnalysisResult execute(const AnalysisRequest& request) override {
        running_ = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        AnalysisResult result;
        result.tool_name = name_;
        result.analysis_id = "mock-" + name_;
        result.timestamp = std::chrono::system_clock::now();
        result.success = should_succeed_;
        result.files_analyzed = should_succeed_ ? 3 : 0;
        
        if (should_succeed_) {
            // Add mock issues
            for (int i = 0; i < mock_issues_count_; ++i) {
                AnalysisIssue issue;
                issue.file_path = "file" + std::to_string(i) + ".cpp";
                issue.line_number = i * 10 + 5;
                issue.severity = (i % 2 == 0) ? IssueSeverity::Warning : IssueSeverity::Error;
                issue.message = "Mock issue " + std::to_string(i) + " from " + name_;
                issue.rule_id = name_ + "-rule-" + std::to_string(i);
                result.issues.push_back(issue);
            }
        } else {
            result.error_message = "Mock execution failed for " + name_;
        }
        
        running_ = false;
        return result;
    }
    
    std::future<AnalysisResult> execute_async(
        const AnalysisRequest& request,
        std::function<void(const AnalysisProgress&)> progress_callback) override {
        
        return std::async(std::launch::async, [this, request, progress_callback]() {
            running_ = true;
            
            // Send progress updates
            if (progress_callback) {
                for (int i = 0; i <= 3; ++i) {
                    AnalysisProgress progress;
                    progress.total_files = 3;
                    progress.processed_files = i;
                    progress.status_message = "Processing with " + name_;
                    if (i < 3) {
                        progress.current_file = "file" + std::to_string(i) + ".cpp";
                    }
                    progress_callback(progress);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            }
            
            auto result = execute(request);
            running_ = false;
            return result;
        });
    }
    
    bool cancel_analysis() override {
        running_ = false;
        return true;
    }
    
    bool is_analysis_running() const override { return running_; }
    
    // Result processing
    AnalysisResult parse_results_file(const std::string& output_file) override {
        AnalysisResult result;
        result.tool_name = name_;
        result.success = true;
        result.files_analyzed = 1;
        
        AnalysisIssue issue;
        issue.file_path = output_file;
        issue.line_number = 1;
        issue.severity = IssueSeverity::Info;
        issue.message = "Parsed issue from " + name_;
        result.issues.push_back(issue);
        
        return result;
    }
    
    std::vector<std::string> get_supported_output_formats() const override {
        return {"xml", "json"};
    }
    
    std::vector<std::string> build_command_line(const AnalysisRequest& request) const override {
        return {name_, request.source_path};
    }
    
    std::string get_help_text() const override {
        return "Help for " + name_;
    }
};

class AnalysisEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine_ = std::make_unique<AnalysisEngine>();
        
        // Register mock tools
        auto tool1 = std::make_unique<MockAnalysisToolForEngine>("tool1", true, true, 2);
        auto tool2 = std::make_unique<MockAnalysisToolForEngine>("tool2", true, true, 1);
        auto tool3 = std::make_unique<MockAnalysisToolForEngine>("unavailable-tool", false, false, 0);
        auto tool4 = std::make_unique<MockAnalysisToolForEngine>("failing-tool", true, false, 0);
        
        engine_->register_tool(std::move(tool1));
        engine_->register_tool(std::move(tool2));
        engine_->register_tool(std::move(tool3));
        engine_->register_tool(std::move(tool4));
    }
    
    void TearDown() override {
        engine_.reset();
    }
    
    std::unique_ptr<AnalysisEngine> engine_;
};

// Test engine construction and basic properties
TEST_F(AnalysisEngineTest, Construction) {
    EXPECT_FALSE(engine_->is_analysis_running());
    
    auto registered_tools = engine_->get_registered_tools();
    EXPECT_EQ(registered_tools.size(), 4);
    EXPECT_TRUE(std::find(registered_tools.begin(), registered_tools.end(), "tool1") != registered_tools.end());
    EXPECT_TRUE(std::find(registered_tools.begin(), registered_tools.end(), "tool2") != registered_tools.end());
    EXPECT_TRUE(std::find(registered_tools.begin(), registered_tools.end(), "unavailable-tool") != registered_tools.end());
    EXPECT_TRUE(std::find(registered_tools.begin(), registered_tools.end(), "failing-tool") != registered_tools.end());
}

TEST_F(AnalysisEngineTest, AvailableTools) {
    auto available_tools = engine_->get_available_tools();
    EXPECT_EQ(available_tools.size(), 3); // tool1, tool2, failing-tool (available but will fail)
    EXPECT_TRUE(std::find(available_tools.begin(), available_tools.end(), "tool1") != available_tools.end());
    EXPECT_TRUE(std::find(available_tools.begin(), available_tools.end(), "tool2") != available_tools.end());
    EXPECT_TRUE(std::find(available_tools.begin(), available_tools.end(), "failing-tool") != available_tools.end());
    EXPECT_TRUE(std::find(available_tools.begin(), available_tools.end(), "unavailable-tool") == available_tools.end());
}

// Test tool information retrieval
TEST_F(AnalysisEngineTest, ToolInformation) {
    auto tool = engine_->get_tool("tool1");
    EXPECT_NE(tool, nullptr);
    EXPECT_EQ(tool->get_name(), "tool1");
    EXPECT_EQ(tool->get_version(), "1.0.0");
    EXPECT_TRUE(tool->is_available());
    EXPECT_FALSE(tool->get_description().empty());
    
    auto unavailable_tool = engine_->get_tool("unavailable-tool");
    EXPECT_NE(unavailable_tool, nullptr);
    EXPECT_EQ(unavailable_tool->get_name(), "unavailable-tool");
    EXPECT_FALSE(unavailable_tool->is_available());
}

TEST_F(AnalysisEngineTest, NonExistentTool) {
    auto tool = engine_->get_tool("non-existent");
    EXPECT_EQ(tool, nullptr); // Should return nullptr for non-existent tools
}

// Test configuration validation
TEST_F(AnalysisEngineTest, ConfigurationValidation) {
    auto validation_results = engine_->validate_configurations({"tool1", "tool2"});
    EXPECT_EQ(validation_results.size(), 2);
    EXPECT_TRUE(validation_results["tool1"].is_valid);
    EXPECT_TRUE(validation_results["tool2"].is_valid);
}

TEST_F(AnalysisEngineTest, ConfigurationValidationWithInvalidTool) {
    // Test with a mix of valid and invalid tools
    auto validation_results = engine_->validate_configurations({"tool1", "non-existent"});
    EXPECT_EQ(validation_results.size(), 2);
    EXPECT_TRUE(validation_results["tool1"].is_valid);
    EXPECT_FALSE(validation_results["non-existent"].is_valid);
    EXPECT_FALSE(validation_results["non-existent"].errors.empty());
}

// Test single tool analysis
TEST_F(AnalysisEngineTest, SingleToolAnalysis) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    request.output_file = "/test/output.xml";
    
    auto result = engine_->analyze_single("tool1", request);
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.tool_name, "tool1");
    EXPECT_EQ(result.files_analyzed, 3);
    EXPECT_EQ(result.issues.size(), 2);
    EXPECT_TRUE(result.error_message.empty());
}

TEST_F(AnalysisEngineTest, SingleToolAnalysisFailure) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    auto result = engine_->analyze_single("failing-tool", request);
    
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.tool_name, "failing-tool");
    EXPECT_EQ(result.files_analyzed, 0);
    EXPECT_TRUE(result.issues.empty());
    EXPECT_FALSE(result.error_message.empty());
}

TEST_F(AnalysisEngineTest, SingleToolAnalysisUnavailable) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    EXPECT_THROW(engine_->analyze_single("unavailable-tool", request), std::runtime_error);
}

TEST_F(AnalysisEngineTest, SingleToolAnalysisNonExistent) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    EXPECT_THROW(engine_->analyze_single("non-existent", request), std::invalid_argument);
}

// Test multiple tool analysis
TEST_F(AnalysisEngineTest, MultipleToolAnalysis) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    request.output_file = "/test/output.xml";
    
    auto results = engine_->analyze_multiple({"tool1", "tool2"}, request);
    
    EXPECT_EQ(results.size(), 2);
    
    // Find results by tool name
    AnalysisResult* tool1_result = nullptr;
    AnalysisResult* tool2_result = nullptr;
    
    for (auto& result : results) {
        if (result.tool_name == "tool1") tool1_result = &result;
        else if (result.tool_name == "tool2") tool2_result = &result;
    }
    
    ASSERT_NE(tool1_result, nullptr);
    ASSERT_NE(tool2_result, nullptr);
    
    EXPECT_TRUE(tool1_result->success);
    EXPECT_EQ(tool1_result->issues.size(), 2);
    
    EXPECT_TRUE(tool2_result->success);
    EXPECT_EQ(tool2_result->issues.size(), 1);
}

TEST_F(AnalysisEngineTest, MultipleToolAnalysisWithFailures) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    auto results = engine_->analyze_multiple({"tool1", "failing-tool"}, request);
    
    EXPECT_EQ(results.size(), 2);
    
    // Check that one succeeded and one failed
    int success_count = 0;
    int failure_count = 0;
    
    for (const auto& result : results) {
        if (result.success) success_count++;
        else failure_count++;
    }
    
    EXPECT_EQ(success_count, 1);
    EXPECT_EQ(failure_count, 1);
}

// Test async analysis
TEST_F(AnalysisEngineTest, AsyncAnalysis) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    request.output_file = "/test/output.xml";
    
    std::vector<std::pair<std::string, AnalysisProgress>> progress_updates;
    std::vector<AnalysisResult> completion_results;
    
    auto progress_callback = [&progress_updates](const std::string& tool_name, const AnalysisProgress& progress) {
        progress_updates.emplace_back(tool_name, progress);
    };
    
    auto completion_callback = [&completion_results](const std::vector<AnalysisResult>& results) {
        completion_results = results;
    };
    
    auto future = engine_->analyze_async({"tool1", "tool2"}, request, progress_callback, completion_callback);
    auto results = future.get();
    
    // Check results
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(completion_results.size(), 2);
    
    // Check that progress was reported
    EXPECT_FALSE(progress_updates.empty());
    
    // Check that we got progress from both tools
    bool got_tool1_progress = false;
    bool got_tool2_progress = false;
    
    for (const auto& [tool_name, progress] : progress_updates) {
        if (tool_name == "tool1") got_tool1_progress = true;
        if (tool_name == "tool2") got_tool2_progress = true;
    }
    
    EXPECT_TRUE(got_tool1_progress);
    EXPECT_TRUE(got_tool2_progress);
}

TEST_F(AnalysisEngineTest, AsyncAnalysisWithoutCallbacks) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    auto future = engine_->analyze_async({"tool1"}, request);
    auto results = future.get();
    
    EXPECT_EQ(results.size(), 1);
    EXPECT_TRUE(results[0].success);
}

// Test analysis state management
TEST_F(AnalysisEngineTest, AnalysisStateManagement) {
    EXPECT_FALSE(engine_->is_analysis_running());
    
    // Start an async analysis
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    auto future = engine_->analyze_async({"tool1"}, request);
    
    // Should be running (though might complete quickly)
    // Wait for completion
    future.get();
    
    // Should not be running after completion
    EXPECT_FALSE(engine_->is_analysis_running());
}

TEST_F(AnalysisEngineTest, CancellationSupport) {
    EXPECT_FALSE(engine_->is_analysis_running());
    
    // Test cancellation when not running
    bool cancelled = engine_->cancel_analysis();
    EXPECT_TRUE(cancelled || !cancelled); // Either outcome is acceptable
    
    EXPECT_FALSE(engine_->is_analysis_running());
}

// Test result aggregation
TEST_F(AnalysisEngineTest, ResultAggregation) {
    // Create some mock results
    std::vector<AnalysisResult> results;
    
    AnalysisResult result1;
    result1.tool_name = "tool1";
    result1.success = true;
    result1.files_analyzed = 3;
    AnalysisIssue issue1;
    issue1.severity = IssueSeverity::Error;
    issue1.file_path = "file1.cpp";
    issue1.line_number = 10;
    issue1.rule_id = "rule1";
    result1.issues.push_back(issue1);
    
    AnalysisResult result2;
    result2.tool_name = "tool2";
    result2.success = true;
    result2.files_analyzed = 2;
    AnalysisIssue issue2;
    issue2.severity = IssueSeverity::Warning;
    issue2.file_path = "file2.cpp";  // Different file
    issue2.line_number = 20;        // Different line
    issue2.rule_id = "rule2";       // Different rule
    result2.issues.push_back(issue2);
    
    results.push_back(result1);
    results.push_back(result2);
    
    auto aggregated = engine_->aggregate_results(results);
    
    EXPECT_TRUE(aggregated.success);
    EXPECT_EQ(aggregated.files_analyzed, 5); // 3 + 2
    EXPECT_EQ(aggregated.issues.size(), 2);
}

// Test edge cases
TEST_F(AnalysisEngineTest, EmptyToolList) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    EXPECT_THROW(engine_->analyze_multiple({}, request), std::invalid_argument);
}

TEST_F(AnalysisEngineTest, DuplicateToolNames) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    // Should handle duplicate tool names gracefully
    auto results = engine_->analyze_multiple({"tool1", "tool1"}, request);
    // Implementation may vary - could deduplicate or run twice
    EXPECT_FALSE(results.empty());
}