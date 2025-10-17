#pragma once

#include <string>
#include <memory>

namespace wip::gui {

/**
 * @brief Base class for all GUI widgets
 * 
 * Provides a common interface for widget lifecycle management including
 * updating state and rendering through ImGui.
 */
class Widget {
public:
    /**
     * @brief Construct a new Widget with an optional ID
     * @param id Unique identifier for this widget (used for ImGui IDs)
     */
    explicit Widget(const std::string& id = "");
    
    /**
     * @brief Virtual destructor for proper polymorphic destruction
     */
    virtual ~Widget() = default;
    
    /**
     * @brief Update widget state and logic
     * 
     * Called every frame before draw() to update widget state,
     * handle input, animations, or other logic updates.
     * 
     * @param delta_time Time elapsed since last update in seconds
     */
    virtual void update([[maybe_unused]] float delta_time) {}
    
    /**
     * @brief Render the widget using ImGui
     * 
     * Called every frame after update() to render the widget.
     * Should contain all ImGui drawing calls for this widget.
     */
    virtual void draw() = 0;
    
    /**
     * @brief Get the widget's unique identifier
     * @return const std::string& Widget ID
     */
    const std::string& get_id() const { return id_; }
    
    /**
     * @brief Set widget visibility
     * @param visible True to show widget, false to hide
     */
    void set_visible(bool visible) { visible_ = visible; }
    
    /**
     * @brief Check if widget is visible
     * @return bool True if widget should be rendered
     */
    bool is_visible() const { return visible_; }
    
    /**
     * @brief Set widget enabled state
     * @param enabled True to enable interactions, false to disable
     */
    void set_enabled(bool enabled) { enabled_ = enabled; }
    
    /**
     * @brief Check if widget is enabled
     * @return bool True if widget accepts interactions
     */
    bool is_enabled() const { return enabled_; }

protected:
    /**
     * @brief Generate a unique ImGui ID for this widget
     * @param suffix Optional suffix to append to the ID
     * @return std::string Unique ImGui ID string
     */
    std::string make_imgui_id(const std::string& suffix = "") const;

private:
    std::string id_;        ///< Unique widget identifier
    bool visible_ = true;   ///< Widget visibility state
    bool enabled_ = true;   ///< Widget enabled state
    
    static int next_id_;    ///< Static counter for generating unique IDs
};

} // namespace wip::gui