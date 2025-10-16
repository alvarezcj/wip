#include "process.h"
#include <cstdio>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <array>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include <filesystem>

namespace wip::utils::process {

// ProcessResult implementation
std::string ProcessResult::combined_output() const {
    std::string result = stdout_output;
    if (!stderr_output.empty()) {
        if (!result.empty() && result.back() != '\n') {
            result += '\n';
        }
        result += stderr_output;
    }
    return result;
}

// ProcessConfig implementation
ProcessConfig ProcessConfig::from_command(const std::string& cmd) {
    ProcessConfig config;
    
    // Simple command parsing - split on spaces (doesn't handle quotes properly)
    // For more complex parsing, we'd need a proper shell parser
    std::istringstream iss(cmd);
    std::string token;
    bool first = true;
    
    while (iss >> token) {
        if (first) {
            config.command = token;
            first = false;
        } else {
            config.arguments.push_back(token);
        }
    }
    
    return config;
}

ProcessConfig ProcessConfig::from_command_args(const std::string& cmd, 
                                              const std::vector<std::string>& args) {
    ProcessConfig config;
    config.command = cmd;
    config.arguments = args;
    return config;
}

// ProcessExecutor implementation
ProcessResult ProcessExecutor::execute(const std::string& command, 
                                      const std::string& working_dir,
                                      std::optional<std::chrono::milliseconds> timeout) {
    auto config = ProcessConfig::from_command(command);
    config.working_directory = working_dir;
    config.timeout = timeout;
    return execute_internal(config);
}

ProcessResult ProcessExecutor::execute(const std::string& command,
                                      const std::vector<std::string>& arguments,
                                      const std::string& working_dir,
                                      std::optional<std::chrono::milliseconds> timeout) {
    auto config = ProcessConfig::from_command_args(command, arguments);
    config.working_directory = working_dir;
    config.timeout = timeout;
    return execute_internal(config);
}

ProcessResult ProcessExecutor::execute(const ProcessConfig& config) {
    return execute_internal(config);
}

std::string ProcessExecutor::execute_and_get_output(const std::string& command) {
    auto result = execute(command);
    if (!result.success()) {
        throw std::runtime_error("Command failed: " + command + 
                                " (exit code: " + std::to_string(result.exit_code) + ")");
    }
    return result.stdout_output;
}

bool ProcessExecutor::command_exists(const std::string& command) {
    return !find_command_path(command).empty();
}

std::string ProcessExecutor::find_command_path(const std::string& command) {
    // Check if command contains path separators
    if (command.find('/') != std::string::npos) {
        return std::filesystem::exists(command) ? command : "";
    }
    
    // Search in PATH
    const char* path_env = std::getenv("PATH");
    if (!path_env) {
        return "";
    }
    
    std::string path_str(path_env);
    std::istringstream path_stream(path_str);
    std::string path_entry;
    
    while (std::getline(path_stream, path_entry, ':')) {
        std::filesystem::path full_path = std::filesystem::path(path_entry) / command;
        if (std::filesystem::exists(full_path) && 
            std::filesystem::is_regular_file(full_path)) {
            // Check if executable
            if (access(full_path.c_str(), X_OK) == 0) {
                return full_path.string();
            }
        }
    }
    
    return "";
}

ProcessResult ProcessExecutor::execute_internal(const ProcessConfig& config) {
    ProcessResult result{};
    auto start_time = std::chrono::steady_clock::now();
    
    // Create pipes for stdout and stderr
    int stdout_pipe[2], stderr_pipe[2];
    
    if (config.capture_stdout && pipe(stdout_pipe) == -1) {
        throw std::runtime_error("Failed to create stdout pipe: " + std::string(strerror(errno)));
    }
    
    if (config.capture_stderr && !config.merge_stderr_to_stdout && pipe(stderr_pipe) == -1) {
        if (config.capture_stdout) {
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
        }
        throw std::runtime_error("Failed to create stderr pipe: " + std::string(strerror(errno)));
    }
    
    // Prepare arguments for execvp
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(config.command.c_str()));
    for (const auto& arg : config.arguments) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);
    
    pid_t pid = fork();
    
    if (pid == -1) {
        // Fork failed
        if (config.capture_stdout) {
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
        }
        if (config.capture_stderr && !config.merge_stderr_to_stdout) {
            close(stderr_pipe[0]);
            close(stderr_pipe[1]);
        }
        throw std::runtime_error("Failed to fork process: " + std::string(strerror(errno)));
    }
    
    if (pid == 0) {
        // Child process
        
        // Change working directory if specified
        if (!config.working_directory.empty()) {
            if (chdir(config.working_directory.c_str()) != 0) {
                _exit(127); // Exit with error code
            }
        }
        
        // Set up pipes
        if (config.capture_stdout) {
            close(stdout_pipe[0]); // Close read end
            dup2(stdout_pipe[1], STDOUT_FILENO);
            close(stdout_pipe[1]);
        }
        
        if (config.capture_stderr) {
            if (config.merge_stderr_to_stdout) {
                dup2(STDOUT_FILENO, STDERR_FILENO);
            } else {
                close(stderr_pipe[0]); // Close read end
                dup2(stderr_pipe[1], STDERR_FILENO);
                close(stderr_pipe[1]);
            }
        }
        
        // Execute the command
        execvp(config.command.c_str(), argv.data());
        
        // If we reach here, execvp failed
        _exit(127);
    }
    
    // Parent process
    
    // Close write ends of pipes
    if (config.capture_stdout) {
        close(stdout_pipe[1]);
    }
    if (config.capture_stderr && !config.merge_stderr_to_stdout) {
        close(stderr_pipe[1]);
    }
    
    // Read output from pipes
    std::string stdout_content, stderr_content;
    
    // Set pipes to non-blocking mode for timeout handling
    if (config.capture_stdout) {
        int flags = fcntl(stdout_pipe[0], F_GETFL);
        fcntl(stdout_pipe[0], F_SETFL, flags | O_NONBLOCK);
    }
    if (config.capture_stderr && !config.merge_stderr_to_stdout) {
        int flags = fcntl(stderr_pipe[0], F_GETFL);
        fcntl(stderr_pipe[0], F_SETFL, flags | O_NONBLOCK);
    }
    
    // Read loop with timeout support
    bool process_finished = false;
    bool timed_out = false;
    
    while (!process_finished && !timed_out) {
        // Check if timeout exceeded
        if (config.timeout.has_value()) {
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            if (elapsed >= config.timeout.value()) {
                timed_out = true;
                kill(pid, SIGTERM); // Try graceful termination first
                usleep(100000); // Wait 100ms
                kill(pid, SIGKILL); // Force kill if still running
                break;
            }
        }
        
        // Check if process has finished
        int status;
        pid_t wait_result = waitpid(pid, &status, WNOHANG);
        if (wait_result == pid) {
            process_finished = true;
            if (WIFEXITED(status)) {
                result.exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                result.exit_code = -WTERMSIG(status);
            }
        }
        
        // Read available data
        if (config.capture_stdout) {
            char buffer[4096];
            ssize_t bytes_read = read(stdout_pipe[0], buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                stdout_content += buffer;
            }
        }
        
        if (config.capture_stderr && !config.merge_stderr_to_stdout) {
            char buffer[4096];
            ssize_t bytes_read = read(stderr_pipe[0], buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                stderr_content += buffer;
            }
        }
        
        // Small sleep to avoid busy waiting
        if (!process_finished) {
            usleep(10000); // 10ms
        }
    }
    
    // Final read to get any remaining data
    if (!timed_out && config.capture_stdout) {
        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(stdout_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            stdout_content += buffer;
        }
    }
    
    if (!timed_out && config.capture_stderr && !config.merge_stderr_to_stdout) {
        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(stderr_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            stderr_content += buffer;
        }
    }
    
    // Close read ends of pipes
    if (config.capture_stdout) {
        close(stdout_pipe[0]);
    }
    if (config.capture_stderr && !config.merge_stderr_to_stdout) {
        close(stderr_pipe[0]);
    }
    
    // Wait for process if it hasn't finished yet (in case of timeout)
    if (!process_finished) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            result.exit_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            result.exit_code = -WTERMSIG(status);
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result.stdout_output = std::move(stdout_content);
    result.stderr_output = std::move(stderr_content);
    result.timed_out = timed_out;
    
    return result;
}

// Convenience functions
namespace convenience {

ProcessResult run(const std::string& command) {
    ProcessExecutor executor;
    return executor.execute(command);
}

std::string get_output(const std::string& command) {
    ProcessExecutor executor;
    return executor.execute_and_get_output(command);
}

bool check(const std::string& command) {
    ProcessExecutor executor;
    return executor.execute(command).success();
}

} // namespace convenience

} // namespace wip::utils::process