#pragma once

#include <process.h>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>

namespace gran_azul::utils {

/**
 * @brief Callback types for async process execution
 */
using ProgressCallback = std::function<void(float progress, const std::string& status)>;
using OutputCallback = std::function<void(const std::string& line)>;
using CompletionCallback = std::function<void(const wip::utils::process::ProcessResult& result)>;

/**
 * @brief Configuration for async process execution
 */
struct AsyncProcessConfig {
    std::string command;
    std::vector<std::string> arguments;
    std::string working_directory;
    
    // Callbacks
    ProgressCallback on_progress;
    OutputCallback on_output;
    CompletionCallback on_completion;
    
    // Progress estimation
    bool enable_progress_estimation = true;
    int estimated_total_files = 0; // For better progress estimation
    
    // Output processing
    bool capture_stdout = true;
    bool capture_stderr = true;
    bool parse_cppcheck_progress = true; // Parse cppcheck-specific progress output
};

/**
 * @brief Async process executor for real-time feedback during analysis
 */
class AsyncProcessExecutor {
public:
    AsyncProcessExecutor();
    ~AsyncProcessExecutor();
    
    // Non-copyable but movable
    AsyncProcessExecutor(const AsyncProcessExecutor&) = delete;
    AsyncProcessExecutor& operator=(const AsyncProcessExecutor&) = delete;
    AsyncProcessExecutor(AsyncProcessExecutor&&) = default;
    AsyncProcessExecutor& operator=(AsyncProcessExecutor&&) = default;
    
    /**
     * @brief Start async execution with callbacks
     * @param config Configuration and callbacks
     * @return Future that completes when process finishes
     */
    std::future<wip::utils::process::ProcessResult> execute_async(const AsyncProcessConfig& config);
    
    /**
     * @brief Cancel the running process
     */
    void cancel();
    
    /**
     * @brief Check if process is currently running
     */
    bool is_running() const;
    
    /**
     * @brief Get current progress (0.0 to 1.0)
     */
    float get_progress() const;
    
    /**
     * @brief Get current status message
     */
    std::string get_status() const;

private:
    std::atomic<bool> is_running_;
    std::atomic<bool> should_cancel_;
    std::atomic<float> current_progress_;
    mutable std::mutex status_mutex_;
    std::string current_status_;
    
    std::thread worker_thread_;
    std::promise<wip::utils::process::ProcessResult> result_promise_;
    
    // Progress estimation for cppcheck
    void parse_cppcheck_output(const std::string& line, const AsyncProcessConfig& config);
    void update_progress(float progress, const std::string& status);
    
    // Platform-specific process management
    void execute_with_realtime_output(const AsyncProcessConfig& config);
    
    #ifdef _WIN32
    void execute_windows(const AsyncProcessConfig& config);
    #else
    void execute_unix(const AsyncProcessConfig& config);
    #endif
};

} // namespace gran_azul::utils