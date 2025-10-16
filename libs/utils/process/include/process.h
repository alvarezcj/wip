#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <optional>

namespace wip::utils::process {

/**
 * @brief Result structure for process execution
 */
struct ProcessResult {
    int exit_code;                    // Process exit code
    std::string stdout_output;        // Standard output content
    std::string stderr_output;        // Standard error content
    std::chrono::milliseconds duration; // Execution duration
    bool timed_out;                   // Whether the process timed out
    
    /**
     * @brief Check if the process executed successfully (exit code 0)
     */
    bool success() const { return exit_code == 0 && !timed_out; }
    
    /**
     * @brief Get combined output (stdout + stderr)
     */
    std::string combined_output() const;
};

/**
 * @brief Configuration for process execution
 */
struct ProcessConfig {
    std::string command;                           // Command to execute
    std::vector<std::string> arguments;            // Command arguments
    std::string working_directory;                 // Working directory (empty = current)
    std::optional<std::chrono::milliseconds> timeout; // Execution timeout
    bool capture_stdout = true;                    // Capture standard output
    bool capture_stderr = true;                    // Capture standard error
    bool merge_stderr_to_stdout = false;           // Redirect stderr to stdout
    
    /**
     * @brief Create config with simple command string
     */
    static ProcessConfig from_command(const std::string& cmd);
    
    /**
     * @brief Create config with command and arguments
     */
    static ProcessConfig from_command_args(const std::string& cmd, 
                                          const std::vector<std::string>& args);
};

/**
 * @brief Utility class for executing system commands and capturing output
 */
class ProcessExecutor {
public:
    ProcessExecutor() = default;
    ~ProcessExecutor() = default;
    
    // Non-copyable but movable
    ProcessExecutor(const ProcessExecutor&) = delete;
    ProcessExecutor& operator=(const ProcessExecutor&) = delete;
    ProcessExecutor(ProcessExecutor&&) = default;
    ProcessExecutor& operator=(ProcessExecutor&&) = default;
    
    /**
     * @brief Execute a simple command string
     * @param command Command string to execute (e.g., "ls -la")
     * @param working_dir Optional working directory
     * @param timeout Optional timeout in milliseconds
     * @return ProcessResult with execution details
     */
    ProcessResult execute(const std::string& command, 
                         const std::string& working_dir = "",
                         std::optional<std::chrono::milliseconds> timeout = std::nullopt);
    
    /**
     * @brief Execute command with separate arguments
     * @param command Base command (e.g., "cppcheck")
     * @param arguments Vector of arguments
     * @param working_dir Optional working directory
     * @param timeout Optional timeout in milliseconds
     * @return ProcessResult with execution details
     */
    ProcessResult execute(const std::string& command,
                         const std::vector<std::string>& arguments,
                         const std::string& working_dir = "",
                         std::optional<std::chrono::milliseconds> timeout = std::nullopt);
    
    /**
     * @brief Execute with detailed configuration
     * @param config Process configuration
     * @return ProcessResult with execution details
     */
    ProcessResult execute(const ProcessConfig& config);
    
    /**
     * @brief Quick utility to execute and get stdout only (throws on error)
     * @param command Command to execute
     * @return stdout content as string
     * @throws std::runtime_error if command fails
     */
    std::string execute_and_get_output(const std::string& command);
    
    /**
     * @brief Check if a command exists and is executable
     * @param command Command name to check
     * @return true if command is available
     */
    static bool command_exists(const std::string& command);
    
    /**
     * @brief Get the full path to a command if it exists
     * @param command Command name to find
     * @return Path to command or empty string if not found
     */
    static std::string find_command_path(const std::string& command);

private:
    ProcessResult execute_internal(const ProcessConfig& config);
};

/**
 * @brief Convenience functions for common operations
 */
namespace convenience {
    /**
     * @brief Execute a simple command and return the result
     */
    ProcessResult run(const std::string& command);
    
    /**
     * @brief Execute command and get stdout (throws on failure)
     */
    std::string get_output(const std::string& command);
    
    /**
     * @brief Check if command succeeds (exit code 0)
     */
    bool check(const std::string& command);
}

} // namespace wip::utils::process