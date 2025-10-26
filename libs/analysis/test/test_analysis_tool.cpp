#include <gtest/gtest.h>
#include "analysis_tool.h"
#include "analysis_types.h"
#include <memory>
#include <future>
#include <chrono>
#include <thread>

using namespace wip::analysis;

// Mock analysis tool for testing
class MockAnalysisTool : public AnalysisTool {
private:
    std::atomic<bool> running_{false};
    std::string mock_name_;
    bool mock_available_;
    bool mock_execution_success_;
    
public:
    MockAnalysisTool(const std::string& name, bool available = true, bool success = true) 
        : mock_name_(name), mock_available_(available), mock_execution_success_(success) {}
    
    // Basic info methods
    std::string get_name() const override { return mock_name_; }
    std::string get_version() const override { return "1.0.0-mock"; }
    std::string get_description() const override { return "Mock tool for testing"; }
    bool is_available() const override { return mock_available_; }
    
    // Required pure virtual methods
    std::vector<std::string> get_supported_extensions() const override {
        return {".cpp", ".h", ".cc", ".hpp"};
    }
    
    // Configuration methods
    void set_configuration(std::unique_ptr<ToolConfig> config) override {
        // Store config (not used in mock)
    }
    
    const ToolConfig* get_configuration() const override {
        return nullptr; // Mock doesn't use config
    }
    
    std::unique_ptr<ToolConfig> create_default_config() const override {
        return nullptr; // Mock doesn't create config
    }
    
    ValidationResult validate_configuration() const override {
        ValidationResult result;
        return result;
    }
    
    // Tool availability methods  
    std::string get_executable_path() const override {
        return "/usr/bin/mock-tool";
    }
    
    std::string get_system_requirements() const override {
        return "Mock tool requires nothing";
    }
    
    // Execution methods
    AnalysisResult execute(const AnalysisRequest& request) override {
        running_ = true;
        
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        AnalysisResult result;
        result.tool_name = get_name();
        result.analysis_id = "mock-" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        result.timestamp = std::chrono::system_clock::now();
        result.success = mock_execution_success_;
        result.files_analyzed = mock_execution_success_ ? 5 : 0;
        
        if (!mock_execution_success_) {
            result.error_message = "Mock execution failed";
        } else {
            // Add some mock issues
            AnalysisIssue issue1;
            issue1.file_path = "test.cpp";
            issue1.line_number = 42;
            issue1.severity = IssueSeverity::Warning;
            issue1.message = "Mock warning";
            issue1.rule_id = "mock-rule-1";
            result.issues.push_back(issue1);
            
            AnalysisIssue issue2;
            issue2.file_path = "test.h";
            issue2.line_number = 10;
            issue2.severity = IssueSeverity::Error;
            issue2.message = "Mock error";
            issue2.rule_id = "mock-rule-2";
            result.issues.push_back(issue2);
        }
        
        running_ = false;
        return result;
    }
    
    std::future<AnalysisResult> execute_async(
        const AnalysisRequest& request,
        std::function<void(const AnalysisProgress&)> progress_callback = nullptr,
        std::function<void(const std::string&)> output_callback = nullptr) override {
        
        return std::async(std::launch::async, [this, request, progress_callback, output_callback]() {
            running_ = true;
            
            // Send initial progress
            if (progress_callback) {
                AnalysisProgress progress;
                progress.total_files = 5;
                progress.processed_files = 0;
                progress.status_message = "Starting " + get_name() + " analysis...";
                progress_callback(progress);
            }
            
            // Send some test output
            if (output_callback) {
                output_callback("Mock analysis starting...");
                output_callback("Analyzing file1.cpp");
                output_callback("Found issue in file1.cpp:10");
            }
            
            // Simulate file processing with progress updates
            for (int i = 0; i <= 5; ++i) {
                if (progress_callback) {
                    AnalysisProgress progress;
                    progress.total_files = 5;
                    progress.processed_files = i;
                    progress.current_file = i < 5 ? ("file" + std::to_string(i) + ".cpp") : "";
                    progress.status_message = i < 5 ? "Processing..." : "Finalizing...";
                    progress_callback(progress);
                }
                
                if (output_callback && i < 5) {
                    output_callback("Processing file" + std::to_string(i) + ".cpp");
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
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
    
    bool is_analysis_running() const override {
        return running_;
    }
    
    // Result processing methods
    AnalysisResult parse_results_file(const std::string& output_file) override {
        AnalysisResult result;
        result.tool_name = get_name();
        result.success = true;
        result.files_analyzed = 1;
        
        // Mock parsing - just create a simple issue
        AnalysisIssue issue;
        issue.file_path = output_file;
        issue.line_number = 1;
        issue.severity = IssueSeverity::Info;
        issue.message = "Parsed from mock file";
        issue.rule_id = "mock-parse";
        result.issues.push_back(issue);
        
        return result;
    }
    
    std::vector<std::string> get_supported_output_formats() const override {
        return {"xml", "json", "text"};
    }
    
    // Utility methods
    std::vector<std::string> build_command_line(const AnalysisRequest& request) const override {
        return {get_name(), request.source_path, "--output=" + request.output_file};
    }
    
    std::string get_help_text() const override {
        return "Mock analysis tool for testing purposes";
    }
};

class AnalysisToolTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_tool_ = std::make_unique<MockAnalysisTool>("mock-tool");
    }
    
    void TearDown() override {
        mock_tool_.reset();
    }
    
    std::unique_ptr<MockAnalysisTool> mock_tool_;
};

// Test basic tool properties
TEST_F(AnalysisToolTest, BasicProperties) {
    EXPECT_EQ(mock_tool_->get_name(), "mock-tool");
    EXPECT_EQ(mock_tool_->get_version(), "1.0.0-mock");
    EXPECT_FALSE(mock_tool_->get_description().empty());
    EXPECT_TRUE(mock_tool_->is_available());
    EXPECT_FALSE(mock_tool_->is_analysis_running());
}

TEST_F(AnalysisToolTest, UnavailableTool) {
    auto unavailable_tool = std::make_unique<MockAnalysisTool>("unavailable-tool", false);
    EXPECT_FALSE(unavailable_tool->is_available());
}

// Test configuration validation
TEST_F(AnalysisToolTest, ConfigValidation) {
    auto validation_result = mock_tool_->validate_configuration();
    EXPECT_TRUE(validation_result.errors.empty());
    EXPECT_TRUE(validation_result.warnings.empty());
}

// Test synchronous execution
TEST_F(AnalysisToolTest, SynchronousExecution) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    request.output_file = "output.xml";
    
    auto result = mock_tool_->execute(request);
    
    EXPECT_EQ(result.tool_name, "mock-tool");
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.analysis_id.empty());
    EXPECT_EQ(result.files_analyzed, 5);
    EXPECT_EQ(result.issues.size(), 2);
    EXPECT_TRUE(result.error_message.empty());
    
    // Check issues
    EXPECT_EQ(result.issues[0].file_path, "test.cpp");
    EXPECT_EQ(result.issues[0].line_number, 42);
    EXPECT_EQ(result.issues[0].severity, IssueSeverity::Warning);
    EXPECT_EQ(result.issues[1].severity, IssueSeverity::Error);
}

TEST_F(AnalysisToolTest, SynchronousExecutionFailure) {
    auto failing_tool = std::make_unique<MockAnalysisTool>("failing-tool", true, false);
    
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    auto result = failing_tool->execute(request);
    
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.files_analyzed, 0);
    EXPECT_TRUE(result.issues.empty());
    EXPECT_FALSE(result.error_message.empty());
}

// Test asynchronous execution
TEST_F(AnalysisToolTest, AsynchronousExecution) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    request.output_file = "output.xml";
    
    std::vector<AnalysisProgress> progress_updates;
    auto progress_callback = [&progress_updates](const AnalysisProgress& progress) {
        progress_updates.push_back(progress);
    };
    
    auto future = mock_tool_->execute_async(request, progress_callback);
    auto result = future.get();
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.tool_name, "mock-tool");
    EXPECT_FALSE(progress_updates.empty());
    
    // Check that progress was reported
    EXPECT_GT(progress_updates.size(), 1);
    EXPECT_EQ(progress_updates[0].processed_files, 0);
    EXPECT_EQ(progress_updates.back().processed_files, 5);
}

TEST_F(AnalysisToolTest, AsynchronousExecutionWithoutCallback) {
    AnalysisRequest request;
    request.source_path = "/test/path";
    
    auto future = mock_tool_->execute_async(request);
    auto result = future.get();
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.tool_name, "mock-tool");
}

// Test cancellation
TEST_F(AnalysisToolTest, CancellationSupport) {
    EXPECT_FALSE(mock_tool_->is_analysis_running());
    
    bool cancelled = mock_tool_->cancel_analysis();
    EXPECT_TRUE(cancelled); // Mock tool supports cancellation
    EXPECT_FALSE(mock_tool_->is_analysis_running());
}

// Test result parsing
TEST_F(AnalysisToolTest, ResultParsing) {
    auto result = mock_tool_->parse_results_file("test_output.xml");
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.tool_name, "mock-tool");
    EXPECT_EQ(result.files_analyzed, 1);
    EXPECT_EQ(result.issues.size(), 1);
    EXPECT_EQ(result.issues[0].file_path, "test_output.xml");
}

// Test supported formats
TEST_F(AnalysisToolTest, SupportedFormats) {
    auto formats = mock_tool_->get_supported_output_formats();
    EXPECT_EQ(formats.size(), 3);
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "xml") != formats.end());
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "json") != formats.end());
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "text") != formats.end());
}

// Test command line building
TEST_F(AnalysisToolTest, CommandLineBuilding) {
    AnalysisRequest request;
    request.source_path = "/test/source";
    request.output_file = "/test/output.xml";
    
    auto cmdline = mock_tool_->build_command_line(request);
    EXPECT_EQ(cmdline.size(), 3);
    EXPECT_EQ(cmdline[0], "mock-tool");
    EXPECT_EQ(cmdline[1], "/test/source");
    EXPECT_EQ(cmdline[2], "--output=/test/output.xml");
}

// Test help text
TEST_F(AnalysisToolTest, HelpText) {
    auto help = mock_tool_->get_help_text();
    EXPECT_FALSE(help.empty());
    EXPECT_TRUE(help.find("Mock") != std::string::npos);
}