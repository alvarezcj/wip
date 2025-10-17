#pragma once

#include "widget.h"
#include <string>
#include <functional>

// Forward declaration for ImGui types
struct ImVec2;
typedef int ImGuiWindowFlags;

namespace wip::gui {

/**
 * @brief Panel class for managing ImGui windows
 * 
 * Provides a higher-level abstraction for ImGui windows with state management,
 * positioning, sizing, and common window operations. Handles the lifecycle
 * of ImGui Begin/End calls and window state.
 */
class Panel : public Widget {
public:
    /**
     * @brief Window position modes
     */
    enum class PositionMode {
        Manual,         ///< Manual positioning via set_position()
        Centered,       ///< Center on screen
        TopLeft,        ///< Top-left corner
        TopRight,       ///< Top-right corner  
        BottomLeft,     ///< Bottom-left corner
        BottomRight     ///< Bottom-right corner
    };
    
    /**
     * @brief Window size modes
     */
    enum class SizeMode {
        Manual,         ///< Manual sizing via set_size()
        Auto,           ///< Auto-size based on content
        FullScreen,     ///< Full viewport size
        Percentage      ///< Percentage of viewport (use set_size_percentage())
    };

public:
    /**
     * @brief Construct a new Panel
     * @param title Window title displayed in title bar
     * @param id Unique identifier (auto-generated if empty)
     */
    explicit Panel(const std::string& title, const std::string& id = "");
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Panel() = default;
    
    /**
     * @brief Update panel state
     * @param delta_time Time elapsed since last update in seconds
     */
    void update(float delta_time) override;
    
    /**
     * @brief Draw the panel window
     * 
     * Handles ImGui::Begin/End calls and delegates content rendering
     * to draw_content() which should be implemented by derived classes.
     */
    void draw() override final;
    
    // Window state management
    
    /**
     * @brief Set window title
     * @param title New window title
     */
    void set_title(const std::string& title) { title_ = title; }
    
    /**
     * @brief Get window title
     * @return const std::string& Current window title
     */
    const std::string& get_title() const { return title_; }
    
    /**
     * @brief Show or hide the panel
     * @param open True to show, false to hide
     */
    void set_open(bool open) { open_ = open; }
    
    /**
     * @brief Check if panel is open
     * @return bool True if panel is open/visible
     */
    bool is_open() const { return open_; }
    
    // Positioning
    
    /**
     * @brief Set window position mode
     * @param mode Position mode to use
     */
    void set_position_mode(PositionMode mode) { position_mode_ = mode; needs_position_update_ = true; }
    
    /**
     * @brief Set manual window position
     * @param x X coordinate
     * @param y Y coordinate
     */
    void set_position(float x, float y);
    
    /**
     * @brief Set position with offset from automatic positioning
     * @param offset_x X offset from automatic position
     * @param offset_y Y offset from automatic position
     */
    void set_position_offset(float offset_x, float offset_y);
    
    // Sizing
    
    /**
     * @brief Set window size mode
     * @param mode Size mode to use
     */
    void set_size_mode(SizeMode mode) { size_mode_ = mode; needs_size_update_ = true; }
    
    /**
     * @brief Set manual window size
     * @param width Window width
     * @param height Window height
     */
    void set_size(float width, float height);
    
    /**
     * @brief Set window size as percentage of viewport
     * @param width_percent Width as percentage (0.0 to 1.0)
     * @param height_percent Height as percentage (0.0 to 1.0)
     */
    void set_size_percentage(float width_percent, float height_percent);
    
    // Window flags and behavior
    
    /**
     * @brief Set ImGui window flags
     * @param flags ImGui window flags
     */
    void set_window_flags(ImGuiWindowFlags flags) { window_flags_ = flags; }
    
    /**
     * @brief Add ImGui window flag
     * @param flag Flag to add
     */
    void add_window_flag(ImGuiWindowFlags flag) { window_flags_ |= flag; }
    
    /**
     * @brief Remove ImGui window flag
     * @param flag Flag to remove
     */
    void remove_window_flag(ImGuiWindowFlags flag) { window_flags_ &= ~flag; }
    
    /**
     * @brief Set if window should auto-resize to content
     * @param auto_resize True to auto-resize
     */
    void set_auto_resize(bool auto_resize);
    
    /**
     * @brief Set if panel should be modal (blocks interaction with other windows)
     * @param modal True for modal behavior
     */
    void set_modal(bool modal) { modal_ = modal; }
    
    /**
     * @brief Check if panel is modal
     * @return bool True if panel is modal
     */
    bool is_modal() const { return modal_; }

protected:
    /**
     * @brief Draw panel content
     * 
     * Override this method to implement panel-specific content.
     * Called between ImGui::Begin() and ImGui::End().
     */
    virtual void draw_content() = 0;
    
    /**
     * @brief Called when panel is about to be drawn
     * 
     * Override to perform setup before ImGui::Begin().
     */
    virtual void on_before_draw() {}
    
    /**
     * @brief Called after panel content is drawn
     * 
     * Override to perform cleanup after content but before ImGui::End().
     */
    virtual void on_after_draw() {}

private:
    /**
     * @brief Update window positioning based on current mode
     */
    void update_position();
    
    /**
     * @brief Update window sizing based on current mode
     */
    void update_size();
    
    /**
     * @brief Get current viewport size
     * @return ImVec2 Viewport size
     */
    ImVec2 get_viewport_size() const;

private:
    // Window state
    std::string title_;
    bool open_ = true;
    bool modal_ = false;
    
    // Positioning
    PositionMode position_mode_ = PositionMode::Manual;
    float position_x_ = 100.0f;
    float position_y_ = 100.0f;
    float position_offset_x_ = 0.0f;
    float position_offset_y_ = 0.0f;
    bool needs_position_update_ = false;
    
    // Sizing  
    SizeMode size_mode_ = SizeMode::Manual;
    float size_width_ = 400.0f;
    float size_height_ = 300.0f;
    float size_width_percent_ = 0.5f;
    float size_height_percent_ = 0.5f;
    bool needs_size_update_ = false;
    
    // ImGui settings
    ImGuiWindowFlags window_flags_ = 0;
    bool first_frame_ = true;
};

} // namespace wip::gui