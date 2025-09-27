#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>

#include <window.h>
#include <event_dispatcher.h>
#include "layer.h"

// Forward declare ImGui context
struct ImGuiContext;

namespace wip::gui::application {

/**
 * @brief Application configuration settings
 */
struct ApplicationConfig {
    std::string name = "WIP Application";
    bool enable_vsync = true;
    bool auto_poll_events = true;
    
    // Default window configuration
    wip::gui::window::WindowConfig default_window_config{};
};

/**
 * @brief Simple application class that manages windows and application lifecycle
 * 
 * The Application class provides an easy-to-use interface for creating and managing
 * multiple windows, handling the main application loop, and coordinating events.
 */
class Application {
public:
    using WindowId = size_t;
    using WindowPtr = std::unique_ptr<wip::gui::window::Window>;
    
    /**
     * @brief Create an application with the specified configuration
     */
    explicit Application(const ApplicationConfig& config = ApplicationConfig{});
    
    /**
     * @brief Create an application with simple name
     */
    explicit Application(std::string_view name);
    
    /**
     * @brief Destructor automatically cleans up all windows
     */
    ~Application();
    
    // Non-copyable but movable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&& other) noexcept;
    Application& operator=(Application&& other) noexcept;
    
    /**
     * @brief Create a new window with the specified configuration
     * @return WindowId for managing the window
     */
    WindowId create_window(const wip::gui::window::WindowConfig& config);
    
    /**
     * @brief Create a new window with simple parameters
     * @return WindowId for managing the window
     */
    WindowId create_window(int width, int height, const std::string& title);
    
    /**
     * @brief Create a new window using default configuration
     * @return WindowId for managing the window
     */
    WindowId create_window();
    
    /**
     * @brief Destroy a window by its ID
     */
    void destroy_window(WindowId window_id);
    
    /**
     * @brief Get a window by its ID
     * @return Pointer to window or nullptr if not found
     */
    wip::gui::window::Window* get_window(WindowId window_id);
    const wip::gui::window::Window* get_window(WindowId window_id) const;
    
    /**
     * @brief Get all active window IDs
     */
    std::vector<WindowId> get_window_ids() const;
    
    /**
     * @brief Get the number of active windows
     */
    size_t window_count() const { return windows_.size(); }
    
    /**
     * @brief Check if the application should quit (all windows closed)
     */
    bool should_quit() const;
    
    /**
     * @brief Request the application to quit
     */
    void quit();
    
    /**
     * @brief Run the main application loop with layers
     * This will keep running until should_quit() returns true.
     * Automatically handles timing, events, updates, and rendering.
     */
    void run();
    
    /**
     * @brief Add a layer to the application
     * Layers are processed in the order they are added.
     * @param layer Unique pointer to the layer (Application takes ownership)
     */
    void add_layer(std::unique_ptr<Layer> layer);
    
    /**
     * @brief Remove a layer by name
     * @param name Name of the layer to remove
     * @return true if layer was found and removed
     */
    bool remove_layer(const std::string& name);
    
    /**
     * @brief Remove a layer by pointer
     * @param layer Pointer to the layer to remove
     * @return true if layer was found and removed
     */
    bool remove_layer(Layer* layer);
    
    /**
     * @brief Get a layer by name
     * @param name Name of the layer to find
     * @return Pointer to layer or nullptr if not found
     */
    Layer* get_layer(const std::string& name);
    const Layer* get_layer(const std::string& name) const;
    
    /**
     * @brief Get all layers
     */
    const std::vector<std::unique_ptr<Layer>>& get_layers() const { return layers_; }
    
    /**
     * @brief Get the number of layers
     */
    size_t layer_count() const { return layers_.size(); }
    
    /**
     * @brief Set target frame rate (0 for unlimited)
     * @param fps Target frames per second
     */
    void set_target_fps(float fps);
    
    /**
     * @brief Get current frame rate
     */
    float get_fps() const { return current_fps_; }
    
    /**
     * @brief Get current frame time in seconds
     */
    float get_frame_time() const { return frame_time_; }
    
    /**
     * @brief Run one iteration of the event loop
     * Useful for custom main loops or integration with other systems
     */
    void poll_events();
    
    /**
     * @brief Wait for events (blocking)
     */
    void wait_events();
    
    /**
     * @brief Set the event dispatcher for all windows
     */
    void set_event_dispatcher(wip::utils::event::EventDispatcher* dispatcher);
    
    /**
     * @brief Get the event dispatcher used by the application
     */
    wip::utils::event::EventDispatcher* get_event_dispatcher() const;
    
    /**
     * @brief Get application configuration
     */
    const ApplicationConfig& get_config() const { return config_; }
    
    /**
     * @brief Get the main window (first created window)
     * @return Pointer to main window or nullptr if no windows exist
     */
    wip::gui::window::Window* get_main_window();
    const wip::gui::window::Window* get_main_window() const;
    
    /**
     * @brief Initialize ImGui for the application
     * Should be called after creating the main window
     */
    void initialize_imgui();
    
    /**
     * @brief Shutdown ImGui
     */
    void shutdown_imgui();
    
    /**
     * @brief Check if ImGui is initialized
     */
    bool is_imgui_initialized() const { return imgui_initialized_; }

private:
    ApplicationConfig config_;
    std::unordered_map<WindowId, WindowPtr> windows_;
    WindowId next_window_id_ = 1;
    WindowId main_window_id_ = 0;
    bool quit_requested_ = false;
    
    wip::utils::event::EventDispatcher* event_dispatcher_;
    bool owns_event_dispatcher_ = false;
    
    // Layer system
    std::vector<std::unique_ptr<Layer>> layers_;
    
    // Timing
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point last_frame_time_;
    float frame_time_ = 0.0f;
    float current_fps_ = 0.0f;
    float target_fps_ = 0.0f;  // 0 = unlimited
    
    // ImGui
    ImGuiContext* imgui_context_ = nullptr;
    bool imgui_initialized_ = false;
    
    void cleanup_closed_windows();
    void setup_layer_event_forwarding();
    void update_layers(Timestep timestep);
    void render_layers(Timestep timestep);
    bool handle_layer_events(const wip::utils::event::Event& event);
    Timestep calculate_timestep();
    void begin_imgui_frame();
    void end_imgui_frame();
};

} // namespace wip::gui::application