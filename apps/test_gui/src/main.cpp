#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstdio>
#include <cmath>

// GLFW
#include <GLFW/glfw3.h>

// OpenGL
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// WIP utilities
#include "wip_string.h"
#include "rng.h"

// Application state
struct AppState {
    bool show_demo_window = false;
    bool show_metrics_window = false;
    bool show_string_utils = false;
    bool show_rng_demo = false;
    
    // String utilities demo state
    char text_input[256] = "Hello, ImGui World!";
    std::string processed_text;
    
    // RNG demo state
    std::vector<float> random_values;
    int random_count = 100;
    float dice_result = 0.0f;
    
    // Graphics demo state
    std::vector<float> sine_wave;
    float animation_time = 0.0f;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};

static AppState g_app_state;
static std::unique_ptr<wip::utils::rng::RandomGenerator> g_rng;

// Error callback for GLFW
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Window close callback
static void window_close_callback(GLFWwindow* window) {
    std::cout << "Window close requested\n";
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// Key callback for global shortcuts
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_F1:
                g_app_state.show_demo_window = !g_app_state.show_demo_window;
                break;
            case GLFW_KEY_F2:
                g_app_state.show_metrics_window = !g_app_state.show_metrics_window;
                break;
        }
    }
}

// Initialize OpenGL context
bool initialize_opengl() {
    // Set OpenGL version (3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Enable forward compatibility on macOS
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    
    return true;
}

// Render the main menu bar
void render_menu_bar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Windows")) {
            ImGui::MenuItem("ImGui Demo", "F1", &g_app_state.show_demo_window);
            ImGui::MenuItem("Metrics", "F2", &g_app_state.show_metrics_window);
            ImGui::Separator();
            ImGui::MenuItem("String Utils Demo", nullptr, &g_app_state.show_string_utils);
            ImGui::MenuItem("RNG Demo", nullptr, &g_app_state.show_rng_demo);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // Could open an about dialog
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
}

// Render string utilities demo window
void render_string_utils_window() {
    if (!g_app_state.show_string_utils) return;
    
    if (ImGui::Begin("String Utilities Demo", &g_app_state.show_string_utils)) {
        ImGui::Text("WIP String Library Integration");
        ImGui::Separator();
        
        // Text input
        ImGui::InputText("Input Text", g_app_state.text_input, sizeof(g_app_state.text_input));
        
        if (ImGui::Button("Process Text")) {
            std::string input(g_app_state.text_input);
            
            // Demonstrate string utilities
            g_app_state.processed_text = "Original: " + input + "\n";
            g_app_state.processed_text += "Upper: " + wip::utils::string::to_upper(input) + "\n";
            g_app_state.processed_text += "Lower: " + wip::utils::string::to_lower(input) + "\n";
            g_app_state.processed_text += "Title: " + wip::utils::string::title_case(input) + "\n";
            g_app_state.processed_text += "Trimmed: '" + wip::utils::string::trim(input) + "'\n";
            
            // Split and rejoin
            auto words = wip::utils::string::split(input, ' ');
            g_app_state.processed_text += "Word count: " + std::to_string(words.size()) + "\n";
            g_app_state.processed_text += "Reversed: " + wip::utils::string::join(std::vector<std::string>(words.rbegin(), words.rend()), " ") + "\n";
            
            // Validation
            g_app_state.processed_text += "Is palindrome: " + std::string(wip::utils::string::is_palindrome(input) ? "Yes" : "No") + "\n";
        }
        
        if (!g_app_state.processed_text.empty()) {
            ImGui::Separator();
            ImGui::TextWrapped("%s", g_app_state.processed_text.c_str());
        }
    }
    ImGui::End();
}

// Render RNG demo window
void render_rng_demo_window() {
    if (!g_app_state.show_rng_demo) return;
    
    if (ImGui::Begin("Random Number Generator Demo", &g_app_state.show_rng_demo)) {
        ImGui::Text("WIP RNG Library Integration");
        ImGui::Separator();
        
        // Generate random values
        ImGui::SliderInt("Count", &g_app_state.random_count, 10, 1000);
        
        if (ImGui::Button("Generate Random Values")) {
            g_app_state.random_values.clear();
            g_app_state.random_values.reserve(g_app_state.random_count);
            
            for (int i = 0; i < g_app_state.random_count; ++i) {
                g_app_state.random_values.push_back(g_rng->uniform_double(0.0, 1.0));
            }
        }
        
        // Dice roll demo
        ImGui::Separator();
        if (ImGui::Button("Roll d20")) {
            g_app_state.dice_result = static_cast<float>(g_rng->uniform_int(1, 20));
        }
        ImGui::SameLine();
        ImGui::Text("Result: %.0f", g_app_state.dice_result);
        
        // Plot random values
        if (!g_app_state.random_values.empty()) {
            ImGui::Separator();
            ImGui::Text("Random Values Distribution:");
            ImGui::PlotHistogram("##histogram", g_app_state.random_values.data(), 
                               static_cast<int>(g_app_state.random_values.size()), 0, nullptr, 0.0f, 1.0f, ImVec2(0, 80));
            
            // Statistics
            float sum = 0.0f;
            for (float val : g_app_state.random_values) sum += val;
            float mean = sum / g_app_state.random_values.size();
            
            ImGui::Text("Mean: %.3f", mean);
            ImGui::Text("Count: %zu", g_app_state.random_values.size());
        }
    }
    ImGui::End();
}

// Render main application window
void render_main_window() {
    // Update animation
    g_app_state.animation_time += ImGui::GetIO().DeltaTime;
    
    // Generate sine wave data
    g_app_state.sine_wave.clear();
    for (int i = 0; i < 100; ++i) {
        float x = i * 0.1f + g_app_state.animation_time;
        g_app_state.sine_wave.push_back(sinf(x));
    }
    
    // Main application window (simplified, no docking)
    if (ImGui::Begin("WIP GUI Test Application", nullptr, ImGuiWindowFlags_MenuBar)) {
        // Menu bar
        render_menu_bar();
        
        ImGui::Text("GLFW + ImGui Desktop Application Demo");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        ImGui::Separator();
        
        // Color picker for background
        ImGui::ColorEdit3("Clear Color", (float*)&g_app_state.clear_color);
        
        ImGui::Separator();
        
        // Animated sine wave
        ImGui::Text("Animated Sine Wave:");
        ImGui::PlotLines("##sine", g_app_state.sine_wave.data(), 
                       static_cast<int>(g_app_state.sine_wave.size()), 0, nullptr, -1.0f, 1.0f, ImVec2(0, 80));
        
        ImGui::Separator();
        
        // Buttons to show demo windows
        if (ImGui::Button("Show ImGui Demo")) {
            g_app_state.show_demo_window = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Show Metrics")) {
            g_app_state.show_metrics_window = true;
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("String Utils Demo")) {
            g_app_state.show_string_utils = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("RNG Demo")) {
            g_app_state.show_rng_demo = true;
        }
        
        ImGui::Separator();
        ImGui::Text("Keyboard Shortcuts:");
        ImGui::Text("  ESC - Exit application");
        ImGui::Text("  F1  - Toggle ImGui Demo");
        ImGui::Text("  F2  - Toggle Metrics Window");
    }
    ImGui::End();
}

int main() {
    std::cout << "Starting GLFW + ImGui Desktop Application...\n";
    
    // Initialize GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    // Initialize OpenGL context settings
    initialize_opengl();
    
    // Create window
    const int window_width = 1280;
    const int window_height = 720;
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "WIP GUI Test Application", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    
    // Set callbacks
    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);
    
    // Make context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Initialize WIP utilities
    g_rng = std::make_unique<wip::utils::rng::RandomGenerator>();
    
    std::cout << "Application initialized successfully!\n";
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLFW Version: " << glfwGetVersionString() << std::endl;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Render our application
        render_main_window();
        
        // Show optional windows
        if (g_app_state.show_demo_window) {
            ImGui::ShowDemoWindow(&g_app_state.show_demo_window);
        }
        
        if (g_app_state.show_metrics_window) {
            ImGui::ShowMetricsWindow(&g_app_state.show_metrics_window);
        }
        
        render_string_utils_window();
        render_rng_demo_window();
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        ImVec4 clear_color = g_app_state.clear_color;
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    std::cout << "Shutting down application...\n";
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "Application shutdown complete.\n";
    return 0;
}