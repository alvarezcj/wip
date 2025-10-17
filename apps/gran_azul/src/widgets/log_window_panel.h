#pragma once

#include <widgets.h>
#include <process.h>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace gran_azul::widgets {

// Log entry structure for the log window
struct LogEntry {
    std::string timestamp;
    std::string command;
    std::string stdout_output;
    std::string stderr_output;
    bool success;
    int exit_code;
    std::chrono::milliseconds duration;
    
    LogEntry(const std::string& cmd, const wip::utils::process::ProcessResult& result) 
        : command(cmd), stdout_output(result.stdout_output), 
          stderr_output(result.stderr_output), success(result.success()), 
          exit_code(result.exit_code), duration(result.duration) {
        // Create timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::ostringstream ss;
        ss << std::put_time(&tm, "%H:%M:%S");
        timestamp = ss.str();
    }
};

class LogWindowPanel : public wip::gui::Panel {
private:
    std::vector<LogEntry> log_entries_;
    bool all_collapsed_;
    bool force_collapse_state_;
    
public:
    LogWindowPanel();
    
    // Panel interface
    void update(float delta_time) override;
    void draw_content() override;
    
    // Log management
    void add_log_entry(const LogEntry& entry);
    void add_log_entry(const std::string& command, const wip::utils::process::ProcessResult& result);
    void clear_log();
    
    // State management
    size_t get_log_count() const { return log_entries_.size(); }
    bool is_empty() const { return log_entries_.empty(); }
    
private:
    void render_header();
    void render_log_entries();
    void render_log_entry(const LogEntry& entry, size_t index, size_t display_number);
    void render_command_section(const LogEntry& entry, size_t index);
    void render_output_section(const LogEntry& entry, size_t index);
};

} // namespace gran_azul::widgets