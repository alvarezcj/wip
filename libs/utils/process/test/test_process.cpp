#include <gtest/gtest.h>
#include "process.h"
#include <chrono>
#include <filesystem>

using namespace wip::utils::process;

class ProcessTest : public ::testing::Test {
protected:
    ProcessExecutor executor;
};

TEST_F(ProcessTest, BasicCommandExecution) {
    std::vector<std::string> args = {"Hello, World!"};
    auto result = executor.execute("echo", args);
    
    EXPECT_TRUE(result.success());
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_FALSE(result.timed_out);
    EXPECT_EQ(result.stdout_output, "Hello, World!\n");
    EXPECT_TRUE(result.stderr_output.empty());
}

TEST_F(ProcessTest, CommandWithStderr) {
    // Use a command that outputs to stderr
    std::vector<std::string> args = {"-c", "echo 'error message' >&2"};
    auto result = executor.execute("sh", args);
    
    EXPECT_TRUE(result.success()); // sh itself succeeds
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_TRUE(result.stdout_output.empty());
    EXPECT_EQ(result.stderr_output, "error message\n");
}

TEST_F(ProcessTest, FailingCommand) {
    auto result = executor.execute("false"); // Command that always fails
    
    EXPECT_FALSE(result.success());
    EXPECT_EQ(result.exit_code, 1);
    EXPECT_FALSE(result.timed_out);
}

TEST_F(ProcessTest, NonExistentCommand) {
    auto result = executor.execute("nonexistent_command_12345");
    
    EXPECT_FALSE(result.success());
    EXPECT_EQ(result.exit_code, 127); // Command not found
}

TEST_F(ProcessTest, WorkingDirectory) {
    // Create a temporary directory for testing
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / "process_test";
    std::filesystem::create_directories(temp_dir);
    
    // Execute pwd in the temp directory
    auto result = executor.execute("pwd", {}, temp_dir.string());
    
    EXPECT_TRUE(result.success());
    EXPECT_EQ(result.stdout_output, temp_dir.string() + "\n");
    
    // Clean up
    std::filesystem::remove_all(temp_dir);
}

TEST_F(ProcessTest, Timeout) {
    // Command that sleeps for 2 seconds, but timeout after 500ms
    std::vector<std::string> args = {"2"};
    auto result = executor.execute("sleep", args, "", std::chrono::milliseconds(500));
    
    EXPECT_FALSE(result.success());
    EXPECT_TRUE(result.timed_out);
    EXPECT_GE(result.duration.count(), 500); // Should take at least 500ms
    EXPECT_LE(result.duration.count(), 1000); // Should timeout before 1 second
}

TEST_F(ProcessTest, CommandExists) {
    EXPECT_TRUE(ProcessExecutor::command_exists("echo"));
    EXPECT_TRUE(ProcessExecutor::command_exists("ls"));
    EXPECT_FALSE(ProcessExecutor::command_exists("nonexistent_command_12345"));
}

TEST_F(ProcessTest, FindCommandPath) {
    std::string echo_path = ProcessExecutor::find_command_path("echo");
    EXPECT_FALSE(echo_path.empty());
    EXPECT_TRUE(std::filesystem::exists(echo_path));
    
    std::string nonexistent = ProcessExecutor::find_command_path("nonexistent_command_12345");
    EXPECT_TRUE(nonexistent.empty());
}

TEST_F(ProcessTest, ConvenienceFunctions) {
    // Test run function
    auto result = convenience::run("echo test");
    EXPECT_TRUE(result.success());
    EXPECT_EQ(result.stdout_output, "test\n");
    
    // Test get_output function
    std::string output = convenience::get_output("echo convenience");
    EXPECT_EQ(output, "convenience\n");
    
    // Test check function
    EXPECT_TRUE(convenience::check("true"));
    EXPECT_FALSE(convenience::check("false"));
}

TEST_F(ProcessTest, CombinedOutput) {
    // Command that outputs to both stdout and stderr
    std::vector<std::string> args = {"-c", "echo 'stdout'; echo 'stderr' >&2"};
    auto result = executor.execute("sh", args);
    
    std::string combined = result.combined_output();
    EXPECT_TRUE(combined.find("stdout") != std::string::npos);
    EXPECT_TRUE(combined.find("stderr") != std::string::npos);
}

TEST_F(ProcessTest, ProcessConfig) {
    // Test ProcessConfig::from_command
    auto config1 = ProcessConfig::from_command("echo hello world");
    EXPECT_EQ(config1.command, "echo");
    EXPECT_EQ(config1.arguments.size(), 2);
    EXPECT_EQ(config1.arguments[0], "hello");
    EXPECT_EQ(config1.arguments[1], "world");
    
    // Test ProcessConfig::from_command_args
    std::vector<std::string> ls_args = {"-la", "/tmp"};
    auto config2 = ProcessConfig::from_command_args("ls", ls_args);
    EXPECT_EQ(config2.command, "ls");
    EXPECT_EQ(config2.arguments.size(), 2);
    EXPECT_EQ(config2.arguments[0], "-la");
    EXPECT_EQ(config2.arguments[1], "/tmp");
}

// Test command with arguments
TEST_F(ProcessTest, CommandWithArguments) {
    if (ProcessExecutor::command_exists("cppcheck")) {
        std::vector<std::string> args = {"--version"};
        auto result = executor.execute("cppcheck", args);
        EXPECT_TRUE(result.success());
        EXPECT_TRUE(result.stdout_output.find("Cppcheck") != std::string::npos);
    } else {
        // Fallback test with ls command
        std::vector<std::string> args = {"--help"};
        auto result = executor.execute("ls", args);
        EXPECT_TRUE(result.success());
        EXPECT_TRUE(result.stdout_output.find("Usage") != std::string::npos || 
                   result.stdout_output.find("--help") != std::string::npos);
    }
}