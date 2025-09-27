#include "application.h"
#include "layer.h"

#include <stdexcept>
#include <algorithm>
#include <thread>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GL/gl.h>

namespace wip::gui::application {

Application::Application(const ApplicationConfig& config) 
    : config_(config), event_dispatcher_(nullptr), owns_event_dispatcher_(true) {
    
    // Create default event dispatcher
    event_dispatcher_ = new wip::utils::event::EventDispatcher();
    
    // Initialize timing
    start_time_ = std::chrono::steady_clock::now();
    last_frame_time_ = start_time_;
}

Application::Application(std::string_view name) : Application(ApplicationConfig{}) {
    config_.name = std::string(name);
}

Application::~Application() {
    // Shutdown ImGui if initialized
    shutdown_imgui();
    
    // Clean up owned event dispatcher
    if (owns_event_dispatcher_ && event_dispatcher_) {
        delete event_dispatcher_;
        event_dispatcher_ = nullptr;
    }
}

Application::Application(Application&& other) noexcept
    : config_(std::move(other.config_)),
      windows_(std::move(other.windows_)),
      next_window_id_(other.next_window_id_),
      main_window_id_(other.main_window_id_),
      quit_requested_(other.quit_requested_),
      event_dispatcher_(other.event_dispatcher_),
      owns_event_dispatcher_(other.owns_event_dispatcher_) {
    
    // Prevent the other object from deleting the dispatcher
    other.event_dispatcher_ = nullptr;
    other.owns_event_dispatcher_ = false;
    other.next_window_id_ = 1;
    other.main_window_id_ = 0;
    other.quit_requested_ = false;
}

Application& Application::operator=(Application&& other) noexcept {
    if (this != &other) {
        // Clean up our resources first
        if (owns_event_dispatcher_ && event_dispatcher_) {
            delete event_dispatcher_;
        }
        
        // Move the data
        config_ = std::move(other.config_);
        windows_ = std::move(other.windows_);
        next_window_id_ = other.next_window_id_;
        main_window_id_ = other.main_window_id_;
        quit_requested_ = other.quit_requested_;
        event_dispatcher_ = other.event_dispatcher_;
        owns_event_dispatcher_ = other.owns_event_dispatcher_;
        
        // Prevent the other object from deleting the dispatcher
        other.event_dispatcher_ = nullptr;
        other.owns_event_dispatcher_ = false;
        other.next_window_id_ = 1;
        other.main_window_id_ = 0;
        other.quit_requested_ = false;
    }
    return *this;
}

Application::WindowId Application::create_window(const wip::gui::window::WindowConfig& config) {
    WindowId id = next_window_id_++;
    
    try {
        auto window = std::make_unique<wip::gui::window::Window>(config);
        window->set_event_dispatcher(event_dispatcher_);
        
        if (config_.enable_vsync) {
            window->make_context_current();
            window->set_vsync(true);
        }
        
        // Store the first window as the main window
        if (main_window_id_ == 0) {
            main_window_id_ = id;
        }
        
        windows_[id] = std::move(window);
        return id;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to create window: " + std::string(e.what()));
    }
}

Application::WindowId Application::create_window(int width, int height, const std::string& title) {
    auto config = config_.default_window_config;
    config.width = width;
    config.height = height;
    config.title = title;
    return create_window(config);
}

Application::WindowId Application::create_window() {
    return create_window(config_.default_window_config);
}

void Application::destroy_window(WindowId window_id) {
    auto it = windows_.find(window_id);
    if (it != windows_.end()) {
        windows_.erase(it);
        
        // If we destroyed the main window, designate a new main window
        if (window_id == main_window_id_ && !windows_.empty()) {
            main_window_id_ = windows_.begin()->first;
        } else if (windows_.empty()) {
            main_window_id_ = 0;
        }
    }
}

wip::gui::window::Window* Application::get_window(WindowId window_id) {
    auto it = windows_.find(window_id);
    return (it != windows_.end()) ? it->second.get() : nullptr;
}

const wip::gui::window::Window* Application::get_window(WindowId window_id) const {
    auto it = windows_.find(window_id);
    return (it != windows_.end()) ? it->second.get() : nullptr;
}

std::vector<Application::WindowId> Application::get_window_ids() const {
    std::vector<WindowId> ids;
    ids.reserve(windows_.size());
    
    for (const auto& pair : windows_) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

bool Application::should_quit() const {
    if (quit_requested_) {
        return true;
    }
    
    // If no windows exist, we should quit
    if (windows_.empty()) {
        return true;
    }
    
    // Check if all windows should close
    for (const auto& pair : windows_) {
        if (!pair.second->should_close()) {
            return false;
        }
    }
    
    return true;
}

void Application::quit() {
    quit_requested_ = true;
}

void Application::run() {
    std::cout << "Starting " << config_.name << "..." << std::endl;
    
    // Attach all layers
    for (auto& layer : layers_) {
        if (layer) {
            layer->on_attach();
        }
    }
    
    // Set up event forwarding to layers
    setup_layer_event_forwarding();
    
    // Main application loop
    while (!should_quit()) {
        std::cout << "." << std::flush;

        // Calculate frame timing
        Timestep timestep = calculate_timestep();
        
        // Poll events first
        if (config_.auto_poll_events) {
            poll_events();
        }
        
        // Clean up any closed windows
        cleanup_closed_windows();
        
        // Update layers
        update_layers(timestep);
        
        // Render layers
        render_layers(timestep);
        
        // Frame rate limiting
        if (target_fps_ > 0.0f) {
            float target_frame_time = 1.0f / target_fps_;
            if (frame_time_ < target_frame_time) {
                float sleep_time = target_frame_time - frame_time_;
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(static_cast<int>(sleep_time * 1000.0f))
                );
            }
        }
        
        // Update FPS counter
        if (frame_time_ > 0.0f) {
            current_fps_ = 1.0f / frame_time_;
        }
    }
    
    // Detach all layers
    for (auto& layer : layers_) {
        if (layer) {
            layer->on_detach();
        }
    }
    
    std::cout << config_.name << " shutting down...\\n";
}

void Application::poll_events() {
    if (config_.auto_poll_events && !windows_.empty()) {
        // Poll events from the first available window
        // GLFW events are global, so polling from any window works
        auto first_window = windows_.begin()->second.get();
        first_window->poll_events();
    }
}

void Application::wait_events() {
    if (!windows_.empty()) {
        // Wait for events using the first available window
        auto first_window = windows_.begin()->second.get();
        first_window->wait_events();
    }
}

void Application::set_event_dispatcher(wip::utils::event::EventDispatcher* dispatcher) {
    if (owns_event_dispatcher_ && event_dispatcher_) {
        delete event_dispatcher_;
        owns_event_dispatcher_ = false;
    }
    
    event_dispatcher_ = dispatcher;
    
    // Update all existing windows
    for (auto& pair : windows_) {
        pair.second->set_event_dispatcher(dispatcher);
    }
}

wip::utils::event::EventDispatcher* Application::get_event_dispatcher() const {
    return event_dispatcher_;
}

wip::gui::window::Window* Application::get_main_window() {
    return get_window(main_window_id_);
}

const wip::gui::window::Window* Application::get_main_window() const {
    return get_window(main_window_id_);
}

void Application::cleanup_closed_windows() {
    // Remove windows that should close
    auto it = windows_.begin();
    while (it != windows_.end()) {
        if (it->second->should_close()) {
            if (it->first == main_window_id_) {
                main_window_id_ = 0;
            }
            it = windows_.erase(it);
        } else {
            ++it;
        }
    }
    
    // If we removed the main window, designate a new one
    if (main_window_id_ == 0 && !windows_.empty()) {
        main_window_id_ = windows_.begin()->first;
    }
}

// Layer management methods
void Application::add_layer(std::unique_ptr<Layer> layer) {
    if (layer) {
        layers_.push_back(std::move(layer));
    }
}

bool Application::remove_layer(const std::string& name) {
    auto it = std::find_if(layers_.begin(), layers_.end(),
        [&name](const std::unique_ptr<Layer>& layer) {
            return layer && layer->get_name() == name;
        });
    
    if (it != layers_.end()) {
        if (*it) {
            (*it)->on_detach();
        }
        layers_.erase(it);
        return true;
    }
    return false;
}

bool Application::remove_layer(Layer* layer_ptr) {
    auto it = std::find_if(layers_.begin(), layers_.end(),
        [layer_ptr](const std::unique_ptr<Layer>& layer) {
            return layer.get() == layer_ptr;
        });
    
    if (it != layers_.end()) {
        if (*it) {
            (*it)->on_detach();
        }
        layers_.erase(it);
        return true;
    }
    return false;
}

Layer* Application::get_layer(const std::string& name) {
    auto it = std::find_if(layers_.begin(), layers_.end(),
        [&name](const std::unique_ptr<Layer>& layer) {
            return layer && layer->get_name() == name;
        });
    
    return (it != layers_.end()) ? it->get() : nullptr;
}

const Layer* Application::get_layer(const std::string& name) const {
    auto it = std::find_if(layers_.begin(), layers_.end(),
        [&name](const std::unique_ptr<Layer>& layer) {
            return layer && layer->get_name() == name;
        });
    
    return (it != layers_.end()) ? it->get() : nullptr;
}

void Application::set_target_fps(float fps) {
    target_fps_ = fps > 0.0f ? fps : 0.0f;
}

// Helper method to forward events from the event dispatcher to layers
void Application::setup_layer_event_forwarding() {
    if (!event_dispatcher_) return;
    
    // Forward all window events to layers
    event_dispatcher_->subscribe<wip::gui::window::events::WindowEvent>(
        [this](const wip::gui::window::events::WindowEvent& event) {
            handle_layer_events(event);
        });
    
    // Forward all keyboard events to layers
    event_dispatcher_->subscribe<wip::gui::window::events::KeyboardEvent>(
        [this](const wip::gui::window::events::KeyboardEvent& event) {
            handle_layer_events(event);
        });
    
    // Forward all character events to layers
    event_dispatcher_->subscribe<wip::gui::window::events::CharacterEvent>(
        [this](const wip::gui::window::events::CharacterEvent& event) {
            handle_layer_events(event);
        });
    
    // Forward all mouse button events to layers
    event_dispatcher_->subscribe<wip::gui::window::events::MouseButtonEvent>(
        [this](const wip::gui::window::events::MouseButtonEvent& event) {
            handle_layer_events(event);
        });
    
    // Forward all mouse move events to layers
    event_dispatcher_->subscribe<wip::gui::window::events::MouseMoveEvent>(
        [this](const wip::gui::window::events::MouseMoveEvent& event) {
            handle_layer_events(event);
        });
    
    // Forward all mouse scroll events to layers
    event_dispatcher_->subscribe<wip::gui::window::events::MouseScrollEvent>(
        [this](const wip::gui::window::events::MouseScrollEvent& event) {
            handle_layer_events(event);
        });
}

// Helper methods for main loop
void Application::update_layers(Timestep timestep) {
    for (auto& layer : layers_) {
        if (layer && layer->is_enabled()) {
            layer->on_update(timestep);
        }
    }
}

void Application::render_layers(Timestep timestep) {
    // Begin ImGui frame if initialized
    begin_imgui_frame();
    
    // Render layers for each window
    for (auto& window_pair : windows_) {
        auto* window = window_pair.second.get();
        if (window && !window->should_close()) {
            // Make this window's context current
            window->make_context_current();
            
            // Render all enabled layers
            for (auto& layer : layers_) {
                if (layer && layer->is_enabled()) {
                    layer->on_render(timestep);
                }
            }
            
            // End ImGui frame and render ImGui content
            end_imgui_frame();
            
            // Swap the buffers to display the rendered frame
            window->swap_buffers();
        }
    }
}

bool Application::handle_layer_events(const wip::utils::event::Event& event) {
    // Events propagate from top to bottom (last added layer first)
    for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
        if (*it && (*it)->is_enabled()) {
            if ((*it)->on_event(event)) {
                return true; // Event was consumed
            }
        }
    }
    return false; // Event was not consumed
}

Timestep Application::calculate_timestep() {
    auto current_time = std::chrono::steady_clock::now();
    
    auto frame_duration = current_time - last_frame_time_;
    frame_time_ = std::chrono::duration<float>(frame_duration).count();
    
    auto total_duration = current_time - start_time_;
    float total_time = std::chrono::duration<float>(total_duration).count();
    
    last_frame_time_ = current_time;
    
    return Timestep(frame_time_, total_time);
}

void Application::initialize_imgui() {
    if (imgui_initialized_) {
        return; // Already initialized
    }
    
    // Get main window for ImGui initialization
    auto* main_window = get_main_window();
    if (!main_window) {
        throw std::runtime_error("Cannot initialize ImGui: no main window available");
    }
    
    // Make the main window context current
    main_window->make_context_current();
    
    // Setup ImGui context
    IMGUI_CHECKVERSION();
    imgui_context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_context_);
    
    // Configure ImGui IO
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable keyboard controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable gamepad controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable multi-viewport / platform windows
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Customize style with modern look
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 4.0f;
    
    // When viewports are enabled we tweak WindowRounding/WindowBg 
    // so platform windows can look identical to regular ones
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    // Customize colors for a modern look
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    
    // Setup platform/renderer backends
    auto* glfw_window = main_window->get_glfw_handle();
    ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    imgui_initialized_ = true;
}

void Application::shutdown_imgui() {
    if (!imgui_initialized_) {
        return; // Not initialized
    }
    
    // Shutdown ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    
    if (imgui_context_) {
        ImGui::DestroyContext(imgui_context_);
        imgui_context_ = nullptr;
    }
    
    imgui_initialized_ = false;
}

void Application::begin_imgui_frame() {
    if (!imgui_initialized_) {
        return;
    }
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Application::end_imgui_frame() {
    if (!imgui_initialized_) {
        return;
    }
    
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Update and render additional platform windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

} // namespace wip::gui::application