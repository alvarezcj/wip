#pragma once

#include <widgets.h>
#include <string>
#include <functional>
#include <chrono>

namespace gran_azul::widgets {

/**
 * @brief Progress dialog for showing real-time analysis feedback
 */
class ProgressDialog : public wip::gui::Widget {
public:
    // Callback types
    using CancelCallback = std::function<void()>;
    using UpdateCallback = std::function<void(float progress, const std::string& status)>;

private:
    std::string title_;
    std::string current_status_;
    std::string output_text_;
    float progress_value_;
    bool is_visible_;
    bool can_cancel_;
    bool is_completed_;
    
    CancelCallback on_cancel_;
    
    // UI state
    bool auto_scroll_output_;
    char output_filter_[256];
    bool auto_close_on_completion_;
    
    // Timing
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point completion_time_;
    std::chrono::steady_clock::time_point last_update_;

public:
    explicit ProgressDialog(const std::string& title = "Progress");
    
    // Widget interface
    void update(float delta_time) override;
    void draw() override;
    
    // Progress control
    void show(const std::string& initial_status = "Starting...");
    void hide();
    void set_progress(float progress, const std::string& status = "");
    void add_output_line(const std::string& line);
    void set_completed(bool success, const std::string& final_message = "");
    
    // State queries
    bool is_visible() const { return is_visible_; }
    bool is_completed() const { return is_completed_; }
    float get_progress() const { return progress_value_; }
    
    // Configuration
    void set_cancellable(bool can_cancel) { can_cancel_ = can_cancel; }
    void set_cancel_callback(CancelCallback callback) { on_cancel_ = callback; }
    void set_auto_close_on_completion(bool auto_close) { auto_close_on_completion_ = auto_close; }
    
    // Output management
    void clear_output();
    const std::string& get_output() const { return output_text_; }
    
private:
    void render_progress_bar();
    void render_status_text();
    void render_output_section();
    void render_control_buttons();
    
    std::string format_elapsed_time() const;
    std::string format_eta(float progress) const;
};

} // namespace gran_azul::widgets