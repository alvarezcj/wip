#include "async_process_executor.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <chrono>
#include <thread>

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#endif

namespace gran_azul::utils {

AsyncProcessExecutor::AsyncProcessExecutor()
    : is_running_(false)
    , should_cancel_(false)
    , current_progress_(0.0f)
{
}

AsyncProcessExecutor::~AsyncProcessExecutor() {
    cancel();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

std::future<wip::utils::process::ProcessResult> AsyncProcessExecutor::execute_async(const AsyncProcessConfig& config) {
    if (is_running_) {
        throw std::runtime_error("Process is already running");
    }
    
    // Clean up previous thread if it exists
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    // Reset state
    is_running_ = true;
    should_cancel_ = false;
    current_progress_ = 0.0f;
    current_status_ = "Starting...";
    
    // Create new promise for this execution
    result_promise_ = std::promise<wip::utils::process::ProcessResult>();
    auto future = result_promise_.get_future();
    
    // Start worker thread
    worker_thread_ = std::thread([this, config]() {
        execute_with_realtime_output(config);
    });
    
    return future;
}

void AsyncProcessExecutor::cancel() {
    should_cancel_ = true;
    
    // Wait for the worker thread to finish with a timeout
    if (worker_thread_.joinable()) {
        // Give it a moment to clean up naturally
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // If it's still running, we'll force join
        if (is_running_) {
            // The thread should detect should_cancel_ and exit soon
            worker_thread_.join();
        }
    }
}

bool AsyncProcessExecutor::is_running() const {
    return is_running_;
}

float AsyncProcessExecutor::get_progress() const {
    return current_progress_;
}

std::string AsyncProcessExecutor::get_status() const {
    std::lock_guard<std::mutex> lock(status_mutex_);
    return current_status_;
}

void AsyncProcessExecutor::update_progress(float progress, const std::string& status) {
    current_progress_ = std::clamp(progress, 0.0f, 1.0f);
    
    {
        std::lock_guard<std::mutex> lock(status_mutex_);
        current_status_ = status;
    }
}

void AsyncProcessExecutor::parse_cppcheck_output(const std::string& line, const AsyncProcessConfig& config) {
    // Parse cppcheck progress output patterns
    static std::regex progress_regex(R"((\d+)/(\d+) files checked (\d+)% done)");
    static std::regex file_regex(R"(Checking (.+\.(cpp|hpp|c|h|cxx|hxx))\.\.\.)");
    static std::regex info_regex(R"(\[info\].*?)");
    
    std::smatch match;
    
    // Check for progress pattern: "X/Y files checked Z% done"
    if (std::regex_search(line, match, progress_regex)) {
        int current_files = std::stoi(match[1].str());
        int total_files = std::stoi(match[2].str());
        int percent = std::stoi(match[3].str());
        
        float progress = percent / 100.0f;
        std::string status = "Checked " + std::to_string(current_files) + "/" + std::to_string(total_files) + " files";
        
        update_progress(progress, status);
        
        if (config.on_progress) {
            config.on_progress(progress, status);
        }
    }
    // Check for file being checked: "Checking filename..."
    else if (std::regex_search(line, match, file_regex)) {
        std::string filename = match[1].str();
        // Extract just the filename without path
        size_t last_slash = filename.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            filename = filename.substr(last_slash + 1);
        }
        
        std::string status = "Checking " + filename;
        update_progress(current_progress_, status);
        
        if (config.on_progress) {
            config.on_progress(current_progress_, status);
        }
    }
    // General info messages
    else if (std::regex_search(line, match, info_regex)) {
        update_progress(current_progress_, line);
        
        if (config.on_progress) {
            config.on_progress(current_progress_, line);
        }
    }
}

void AsyncProcessExecutor::execute_with_realtime_output(const AsyncProcessConfig& config) {
    try {
        #ifdef _WIN32
        execute_windows(config);
        #else
        execute_unix(config);
        #endif
    } catch (const std::exception& e) {
        // Create error result
        wip::utils::process::ProcessResult error_result;
        error_result.exit_code = -1;
        error_result.stderr_output = e.what();
        error_result.timed_out = false;
        error_result.duration = std::chrono::milliseconds(0);
        
        try {
            result_promise_.set_value(error_result);
        } catch (const std::exception&) {
            // Promise might already be set, ignore
        }
    }
    
    // Ensure state is properly reset
    is_running_ = false;
    should_cancel_ = false;
}

#ifndef _WIN32
void AsyncProcessExecutor::execute_unix(const AsyncProcessConfig& config) {
    auto start_time = std::chrono::steady_clock::now();
    
    // Build command string
    std::string full_command = config.command;
    for (const auto& arg : config.arguments) {
        full_command += " " + arg;
    }
    
    // Create pipes for stdout and stderr
    int stdout_pipe[2];
    int stderr_pipe[2];
    
    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
        throw std::runtime_error("Failed to create pipes");
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("Failed to fork process");
    }
    
    if (pid == 0) {
        // Child process
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        
        // Change working directory if specified
        if (!config.working_directory.empty()) {
            chdir(config.working_directory.c_str());
        }
        
        // Execute command
        execl("/bin/sh", "sh", "-c", full_command.c_str(), nullptr);
        exit(127); // If execl fails
    } else {
        // Parent process
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        
        // Make pipes non-blocking
        fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
        fcntl(stderr_pipe[0], F_SETFL, O_NONBLOCK);
        
        std::string stdout_output;
        std::string stderr_output;
        std::string stdout_buffer;
        std::string stderr_buffer;
        
        bool process_running = true;
        
        while (process_running || !stdout_buffer.empty() || !stderr_buffer.empty()) {
            if (should_cancel_) {
                kill(pid, SIGTERM);
                break;
            }
            
            // Check if process is still running
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                process_running = false;
            }
            
            // Poll for output
            struct pollfd fds[2];
            fds[0].fd = stdout_pipe[0];
            fds[0].events = POLLIN;
            fds[1].fd = stderr_pipe[0];
            fds[1].events = POLLIN;
            
            int poll_result = poll(fds, 2, 100); // 100ms timeout
            
            if (poll_result > 0) {
                // Read stdout
                if (fds[0].revents & POLLIN) {
                    char buffer[4096];
                    ssize_t bytes = read(stdout_pipe[0], buffer, sizeof(buffer) - 1);
                    if (bytes > 0) {
                        buffer[bytes] = '\0';
                        stdout_buffer += buffer;
                        
                        // Process complete lines
                        size_t pos;
                        while ((pos = stdout_buffer.find('\n')) != std::string::npos) {
                            std::string line = stdout_buffer.substr(0, pos);
                            stdout_buffer.erase(0, pos + 1);
                            
                            stdout_output += line + "\n";
                            
                            if (config.on_output) {
                                config.on_output(line);
                            }
                            
                            if (config.parse_cppcheck_progress) {
                                parse_cppcheck_output(line, config);
                            }
                        }
                    }
                }
                
                // Read stderr
                if (fds[1].revents & POLLIN) {
                    char buffer[4096];
                    ssize_t bytes = read(stderr_pipe[0], buffer, sizeof(buffer) - 1);
                    if (bytes > 0) {
                        buffer[bytes] = '\0';
                        stderr_buffer += buffer;
                        
                        // Process complete lines
                        size_t pos;
                        while ((pos = stderr_buffer.find('\n')) != std::string::npos) {
                            std::string line = stderr_buffer.substr(0, pos);
                            stderr_buffer.erase(0, pos + 1);
                            
                            stderr_output += line + "\n";
                            
                            if (config.on_output) {
                                config.on_output("ERROR: " + line);
                            }
                        }
                    }
                }
            }
        }
        
        // Close pipes
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        
        // Wait for final process status
        int final_status;
        waitpid(pid, &final_status, 0);
        
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // Create result
        wip::utils::process::ProcessResult result;
        result.exit_code = WIFEXITED(final_status) ? WEXITSTATUS(final_status) : -1;
        result.stdout_output = stdout_output;
        result.stderr_output = stderr_output;
        result.duration = duration;
        result.timed_out = should_cancel_;
        
        // Final progress update
        if (config.on_progress) {
            config.on_progress(1.0f, result.success() ? "Completed successfully" : "Completed with errors");
        }
        
        if (config.on_completion) {
            config.on_completion(result);
        }
        
        // Set the result promise (protect against multiple calls)
        try {
            result_promise_.set_value(result);
        } catch (const std::exception&) {
            // Promise might already be set, ignore
        }
    }
}
#endif

#ifdef _WIN32
void AsyncProcessExecutor::execute_windows(const AsyncProcessConfig& config) {
    // Windows implementation would go here
    // For now, fall back to synchronous execution
    wip::utils::process::ProcessExecutor executor;
    auto result = executor.execute(config.command, config.arguments, config.working_directory);
    
    if (config.on_completion) {
        config.on_completion(result);
    }
    
    result_promise_.set_value(result);
}
#endif

} // namespace gran_azul::utils