#pragma once

#include <event.h>
#include <string>

namespace wip::gui::application {

/**
 * @brief Timing information for frame-based updates
 */
struct Timestep {
    float delta_time = 0.0f;  // Time since last frame (seconds)
    float total_time = 0.0f;  // Total application time (seconds)
    
    Timestep(float dt = 0.0f, float total = 0.0f) : delta_time(dt), total_time(total) {}
    
    // Convenient conversion operators
    operator float() const { return delta_time; }
    float milliseconds() const { return delta_time * 1000.0f; }
    float seconds() const { return delta_time; }
};

/**
 * @brief Base class for application layers
 * 
 * Layers provide a way to organize application logic into separate, reusable components.
 * Each layer can handle events, update logic, and render content independently.
 * Layers are processed in order: events propagate from top to bottom, 
 * updates run bottom to top, and rendering happens bottom to top.
 */
class Layer {
public:
    /**
     * @brief Create a layer with the given name
     */
    explicit Layer(std::string_view name = "Layer");
    
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Layer() = default;
    
    /**
     * @brief Called when the layer is attached to the application
     * Use this for initialization that requires the application context
     */
    virtual void on_attach() {}
    
    /**
     * @brief Called when the layer is detached from the application
     * Use this for cleanup
     */
    virtual void on_detach() {}
    
    /**
     * @brief Called every frame to update layer logic
     * @param timestep Frame timing information
     */
    virtual void on_update(Timestep timestep) {}
    
    /**
     * @brief Called every frame to render layer content
     * @param timestep Frame timing information for smooth animations
     */
    virtual void on_render(Timestep timestep) {}
    
    /**
     * @brief Called when an event occurs
     * @param event The event to handle
     * @return true if the event was consumed and should not propagate further
     */
    virtual bool on_event(const wip::utils::event::Event& event) { return false; }
    
    /**
     * @brief Get the layer name
     */
    const std::string& get_name() const { return name_; }
    
    /**
     * @brief Check if the layer is enabled
     * Disabled layers don't receive updates, renders, or events
     */
    bool is_enabled() const { return enabled_; }
    
    /**
     * @brief Enable or disable the layer
     */
    void set_enabled(bool enabled) { enabled_ = enabled; }
    
    /**
     * @brief Enable the layer
     */
    void enable() { enabled_ = true; }
    
    /**
     * @brief Disable the layer
     */
    void disable() { enabled_ = false; }

protected:
    std::string name_;
    bool enabled_ = true;
};

} // namespace wip::gui::application