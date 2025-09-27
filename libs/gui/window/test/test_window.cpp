#include <gtest/gtest.h>
#include <window.h>
#include <thread>
#include <chrono>

using namespace wip::gui::window;

class WindowTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Most tests will create windows in a headless environment
        // We need to check if we can actually create windows
    }
    
    void TearDown() override {
        // Clean up any created windows
    }
    
    // Helper function to check if we can create windows (not in CI/headless)
    bool can_create_window() {
        try {
            WindowConfig config;
            config.visible = false; // Don't show window in tests
            Window test_window(config);
            return true;
        } catch (const std::runtime_error&) {
            return false;
        }
    }
};

TEST_F(WindowTest, GLFWInitialization) {
    // Test GLFW initialization
    if (can_create_window()) {
        EXPECT_TRUE(Window::is_glfw_initialized());
        EXPECT_FALSE(Window::get_glfw_version().empty());
    } else {
        GTEST_SKIP() << "Cannot create windows in this environment (headless/CI)";
    }
}

TEST_F(WindowTest, WindowCreation) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.width = 640;
    config.height = 480;
    config.title = "Test Window";
    config.visible = false;
    
    EXPECT_NO_THROW({
        Window window(config);
        EXPECT_NE(window.get_glfw_handle(), nullptr);
    });
}

TEST_F(WindowTest, WindowSize) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.width = 800;
    config.height = 600;
    config.visible = false;
    
    Window window(config);
    
    auto [width, height] = window.get_size();
    EXPECT_EQ(width, 800);
    EXPECT_EQ(height, 600);
    
    // Test resizing
    window.set_size(1024, 768);
    auto [new_width, new_height] = window.get_size();
    EXPECT_EQ(new_width, 1024);
    EXPECT_EQ(new_height, 768);
}

TEST_F(WindowTest, WindowPosition) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    // Test setting position
    window.set_position(100, 200);
    auto [x, y] = window.get_position();
    
    // Position might be adjusted by window manager, so just check it's reasonable
    EXPECT_GE(x, 0);
    EXPECT_GE(y, 0);
}

TEST_F(WindowTest, WindowVisibility) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    EXPECT_FALSE(window.is_visible());
    
    window.show();
    // Note: Visibility might not change immediately in headless environment
    
    window.hide();
    // Same note applies
}

TEST_F(WindowTest, ShouldClose) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    EXPECT_FALSE(window.should_close());
    
    window.set_should_close(true);
    EXPECT_TRUE(window.should_close());
    
    window.set_should_close(false);
    EXPECT_FALSE(window.should_close());
}

TEST_F(WindowTest, FramebufferSize) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.width = 640;
    config.height = 480;
    config.visible = false;
    Window window(config);
    
    auto [fb_width, fb_height] = window.get_framebuffer_size();
    
    // Framebuffer size should be at least the window size
    // (might be larger on high-DPI displays)
    EXPECT_GE(fb_width, 640);
    EXPECT_GE(fb_height, 480);
}

TEST_F(WindowTest, CursorPosition) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    // Test setting cursor position
    window.set_cursor_position(100.0, 200.0);
    auto [x, y] = window.get_cursor_position();
    
    // Allow for some floating point precision differences
    EXPECT_NEAR(x, 100.0, 1.0);
    EXPECT_NEAR(y, 200.0, 1.0);
}

TEST_F(WindowTest, EventDispatcherManagement) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    // Should use global dispatcher by default
    EXPECT_EQ(window.get_event_dispatcher(), &wip::utils::event::global_dispatcher());
    
    // Test custom dispatcher
    wip::utils::event::EventDispatcher custom_dispatcher;
    window.set_event_dispatcher(&custom_dispatcher);
    EXPECT_EQ(window.get_event_dispatcher(), &custom_dispatcher);
    
    // Test nullptr falls back to global
    window.set_event_dispatcher(nullptr);
    EXPECT_EQ(window.get_event_dispatcher(), &wip::utils::event::global_dispatcher());
}

TEST_F(WindowTest, MoveConstructor) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.visible = false;
    config.width = 640;
    config.height = 480;
    
    Window window1(config);
    GLFWwindow* handle = window1.get_glfw_handle();
    EXPECT_NE(handle, nullptr);
    
    // Move construct
    Window window2(std::move(window1));
    EXPECT_EQ(window2.get_glfw_handle(), handle);
    EXPECT_EQ(window1.get_glfw_handle(), nullptr);
    
    auto [width, height] = window2.get_size();
    EXPECT_EQ(width, 640);
    EXPECT_EQ(height, 480);
}

TEST_F(WindowTest, MoveAssignment) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config1, config2;
    config1.visible = false;
    config1.width = 640;
    config1.height = 480;
    config2.visible = false;
    config2.width = 800;
    config2.height = 600;
    
    Window window1(config1);
    Window window2(config2);
    
    GLFWwindow* handle1 = window1.get_glfw_handle();
    
    // Move assign
    window2 = std::move(window1);
    
    EXPECT_EQ(window2.get_glfw_handle(), handle1);
    EXPECT_EQ(window1.get_glfw_handle(), nullptr);
    
    auto [width, height] = window2.get_size();
    EXPECT_EQ(width, 640);
    EXPECT_EQ(height, 480);
}

TEST_F(WindowTest, SimpleConstructor) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    // Test the simple constructor
    Window window(1024, 768, "Simple Test Window");
    auto [width, height] = window.get_size();
    EXPECT_EQ(width, 1024);
    EXPECT_EQ(height, 768);
}

TEST_F(WindowTest, WindowStates) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    // Test iconify/restore
    EXPECT_FALSE(window.is_iconified());
    window.iconify();
    // Note: State changes might not be immediate in test environment
    
    window.restore();
    
    // Test maximize
    EXPECT_FALSE(window.is_maximized());
    window.maximize();
    // Note: Maximize might not work in headless environment
}

TEST_F(WindowTest, GLFWVersionInfo) {
    std::string version = Window::get_glfw_version();
    EXPECT_FALSE(version.empty());
    
    // Should be in format "major.minor.revision"
    size_t first_dot = version.find('.');
    size_t second_dot = version.find('.', first_dot + 1);
    
    EXPECT_NE(first_dot, std::string::npos);
    EXPECT_NE(second_dot, std::string::npos);
    EXPECT_GT(second_dot, first_dot);
}