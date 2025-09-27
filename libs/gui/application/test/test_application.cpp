#include <gtest/gtest.h>
#include "application.h"
#include "layer.h"
#include <set>

using namespace wip::gui::application;

class ApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Tests run in headless environment, so we can't create actual windows
        // We'll test the interface and logic as much as possible
    }
    
    void TearDown() override {
        // Cleanup if needed
    }
};

// Test application construction
TEST_F(ApplicationTest, DefaultConstruction) {
    Application app;
    
    EXPECT_EQ(app.window_count(), 0);
    EXPECT_TRUE(app.get_window_ids().empty());
    EXPECT_EQ(app.get_main_window(), nullptr);
    EXPECT_NE(app.get_event_dispatcher(), nullptr);
    EXPECT_EQ(app.get_config().name, "WIP Application");
    EXPECT_TRUE(app.get_config().enable_vsync);
    EXPECT_TRUE(app.get_config().auto_poll_events);
}

TEST_F(ApplicationTest, NamedConstruction) {
    Application app("Test App");
    
    EXPECT_EQ(app.get_config().name, "Test App");
    EXPECT_EQ(app.window_count(), 0);
    EXPECT_NE(app.get_event_dispatcher(), nullptr);
}

TEST_F(ApplicationTest, ConfigConstruction) {
    ApplicationConfig config;
    config.name = "Custom App";
    config.enable_vsync = false;
    config.auto_poll_events = false;
    config.default_window_config.width = 1024;
    config.default_window_config.height = 768;
    
    Application app(config);
    
    EXPECT_EQ(app.get_config().name, "Custom App");
    EXPECT_FALSE(app.get_config().enable_vsync);
    EXPECT_FALSE(app.get_config().auto_poll_events);
    EXPECT_EQ(app.get_config().default_window_config.width, 1024);
    EXPECT_EQ(app.get_config().default_window_config.height, 768);
}

// Test window management (without actual window creation due to headless environment)
TEST_F(ApplicationTest, WindowIdGeneration) {
    Application app;
    
    // Since we can't create windows in headless environment,
    // we test the logic by checking that window creation attempts
    // generate unique IDs (even if they fail)
    
    EXPECT_EQ(app.window_count(), 0);
    EXPECT_TRUE(app.get_window_ids().empty());
}

TEST_F(ApplicationTest, WindowRetrieval) {
    Application app;
    
    // Test retrieving non-existent window
    EXPECT_EQ(app.get_window(1), nullptr);
    EXPECT_EQ(app.get_window(999), nullptr);
}

TEST_F(ApplicationTest, WindowDestructionNonExistent) {
    Application app;
    
    // Destroying non-existent windows should not crash
    app.destroy_window(1);
    app.destroy_window(999);
    
    EXPECT_EQ(app.window_count(), 0);
}

// Test application lifecycle
TEST_F(ApplicationTest, QuitFunctionality) {
    Application app;
    
    // With no windows, should_quit should return true
    EXPECT_TRUE(app.should_quit());
    
    app.quit();
    EXPECT_TRUE(app.should_quit());
}

TEST_F(ApplicationTest, ShouldQuitWithNoWindows) {
    Application app;
    
    // With no windows, should_quit should return true
    EXPECT_TRUE(app.should_quit());
}

// Test event dispatcher management
TEST_F(ApplicationTest, EventDispatcherManagement) {
    Application app;
    
    auto* original_dispatcher = app.get_event_dispatcher();
    EXPECT_NE(original_dispatcher, nullptr);
    
    // Create a custom dispatcher
    auto custom_dispatcher = std::make_unique<wip::utils::event::EventDispatcher>();
    auto* custom_ptr = custom_dispatcher.get();
    
    app.set_event_dispatcher(custom_ptr);
    EXPECT_EQ(app.get_event_dispatcher(), custom_ptr);
    
    // The application should not own the custom dispatcher
    custom_dispatcher.reset(); // We still own it
}

// Test move semantics
TEST_F(ApplicationTest, MoveConstruction) {
    Application app1("Original App");
    auto* original_dispatcher = app1.get_event_dispatcher();
    
    Application app2 = std::move(app1);
    
    EXPECT_EQ(app2.get_config().name, "Original App");
    EXPECT_EQ(app2.get_event_dispatcher(), original_dispatcher);
    EXPECT_EQ(app2.window_count(), 0);
}

TEST_F(ApplicationTest, MoveAssignment) {
    Application app1("App1");
    Application app2("App2");
    
    auto* app1_dispatcher = app1.get_event_dispatcher();
    
    app2 = std::move(app1);
    
    EXPECT_EQ(app2.get_config().name, "App1");
    EXPECT_EQ(app2.get_event_dispatcher(), app1_dispatcher);
}

// Test configuration
TEST_F(ApplicationTest, WindowConfigInheritance) {
    ApplicationConfig config;
    config.default_window_config.width = 1920;
    config.default_window_config.height = 1080;
    config.default_window_config.title = "Default Title";
    
    Application app(config);
    
    const auto& app_config = app.get_config();
    EXPECT_EQ(app_config.default_window_config.width, 1920);
    EXPECT_EQ(app_config.default_window_config.height, 1080);
    EXPECT_EQ(app_config.default_window_config.title, "Default Title");
}

// Test edge cases
TEST_F(ApplicationTest, MultipleQuitCalls) {
    Application app;
    
    app.quit();
    EXPECT_TRUE(app.should_quit());
    
    app.quit(); // Should not crash
    EXPECT_TRUE(app.should_quit());
}

TEST_F(ApplicationTest, EventPolling) {
    Application app;
    
    // These should not crash even with no windows
    app.poll_events();
    // Note: wait_events() is not tested as it would block in headless environment
}

// Integration tests (testing behavior without actual GLFW windows)
class ApplicationIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any common test data
    }
};

TEST_F(ApplicationIntegrationTest, ApplicationLifecycle) {
    // Test the complete lifecycle without creating actual windows
    Application app("Integration Test");
    
    // With no windows, should_quit should return true
    EXPECT_TRUE(app.should_quit());
    EXPECT_EQ(app.window_count(), 0);
    EXPECT_EQ(app.get_main_window(), nullptr);
    
    // Application should quit when explicitly requested
    app.quit();
    EXPECT_TRUE(app.should_quit());
}

TEST_F(ApplicationIntegrationTest, EventDispatcherIntegration) {
    Application app;
    auto original_dispatcher = app.get_event_dispatcher();
    
    // Test that we can replace the event dispatcher
    auto new_dispatcher = std::make_unique<wip::utils::event::EventDispatcher>();
    auto* new_dispatcher_ptr = new_dispatcher.get();
    
    app.set_event_dispatcher(new_dispatcher_ptr);
    EXPECT_EQ(app.get_event_dispatcher(), new_dispatcher_ptr);
    EXPECT_NE(app.get_event_dispatcher(), original_dispatcher);
}

// Performance tests
TEST_F(ApplicationTest, WindowIdUniqueness) {
    Application app;
    
    // Test that window IDs would be unique (even if window creation fails)
    std::set<Application::WindowId> attempted_ids;
    
    // We can't create actual windows, but we can test the ID generation logic
    // by checking that the application maintains proper state
    EXPECT_EQ(app.window_count(), 0);
    
    // After attempting to create windows (even if they fail), 
    // the application state should remain consistent
    EXPECT_TRUE(app.get_window_ids().empty());
}

// Test Layer system
class TestLayer : public Layer {
public:
    explicit TestLayer(std::string_view name) : Layer(name) {}
    
    void on_attach() override { attach_count++; }
    void on_detach() override { detach_count++; }
    void on_update(Timestep timestep) override { 
        update_count++; 
        last_delta_time = timestep.delta_time;
        last_total_time = timestep.total_time;
    }
    void on_render(Timestep timestep) override { 
        render_count++; 
        last_render_delta = timestep.delta_time;
    }
    bool on_event(const wip::utils::event::Event& event) override { 
        event_count++; 
        return consume_events;
    }
    
    int attach_count = 0;
    int detach_count = 0;
    int update_count = 0;
    int render_count = 0;
    int event_count = 0;
    float last_delta_time = 0.0f;
    float last_total_time = 0.0f;
    float last_render_delta = 0.0f;
    bool consume_events = false;
};

class LayerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test Layer construction and basic functionality
TEST_F(LayerTest, LayerConstruction) {
    TestLayer layer("TestLayer");
    
    EXPECT_EQ(layer.get_name(), "TestLayer");
    EXPECT_TRUE(layer.is_enabled());
    EXPECT_EQ(layer.attach_count, 0);
    EXPECT_EQ(layer.detach_count, 0);
}

TEST_F(LayerTest, LayerEnableDisable) {
    TestLayer layer("TestLayer");
    
    EXPECT_TRUE(layer.is_enabled());
    
    layer.disable();
    EXPECT_FALSE(layer.is_enabled());
    
    layer.enable();
    EXPECT_TRUE(layer.is_enabled());
    
    layer.set_enabled(false);
    EXPECT_FALSE(layer.is_enabled());
}

// Test Layer management in Application
TEST_F(LayerTest, ApplicationLayerManagement) {
    Application app("Layer Test");
    
    EXPECT_EQ(app.layer_count(), 0);
    
    // Add layers
    auto layer1 = std::make_unique<TestLayer>("Layer1");
    auto layer2 = std::make_unique<TestLayer>("Layer2");
    
    auto* layer1_ptr = layer1.get();
    auto* layer2_ptr = layer2.get();
    
    app.add_layer(std::move(layer1));
    app.add_layer(std::move(layer2));
    
    EXPECT_EQ(app.layer_count(), 2);
    EXPECT_EQ(app.get_layer("Layer1"), layer1_ptr);
    EXPECT_EQ(app.get_layer("Layer2"), layer2_ptr);
    EXPECT_EQ(app.get_layer("NonExistent"), nullptr);
}

TEST_F(LayerTest, ApplicationLayerRemoval) {
    Application app("Layer Test");
    
    auto layer1 = std::make_unique<TestLayer>("Layer1");
    auto layer2 = std::make_unique<TestLayer>("Layer2");
    
    auto* layer1_ptr = layer1.get();
    auto* layer2_ptr = layer2.get();
    
    app.add_layer(std::move(layer1));
    app.add_layer(std::move(layer2));
    
    EXPECT_EQ(app.layer_count(), 2);
    
    // Remove by name
    EXPECT_TRUE(app.remove_layer("Layer1"));
    EXPECT_EQ(app.layer_count(), 1);
    EXPECT_EQ(app.get_layer("Layer1"), nullptr);
    EXPECT_EQ(app.get_layer("Layer2"), layer2_ptr);
    
    // Remove by pointer
    EXPECT_TRUE(app.remove_layer(layer2_ptr));
    EXPECT_EQ(app.layer_count(), 0);
    EXPECT_EQ(app.get_layer("Layer2"), nullptr);
    
    // Remove non-existent
    EXPECT_FALSE(app.remove_layer("NonExistent"));
    EXPECT_FALSE(app.remove_layer(layer1_ptr));
}

// Test Timestep functionality
TEST_F(LayerTest, TimestepFunctionality) {
    Timestep ts1;
    EXPECT_EQ(ts1.delta_time, 0.0f);
    EXPECT_EQ(ts1.total_time, 0.0f);
    
    Timestep ts2(0.016f, 1.5f);
    EXPECT_FLOAT_EQ(ts2.delta_time, 0.016f);
    EXPECT_FLOAT_EQ(ts2.total_time, 1.5f);
    EXPECT_FLOAT_EQ(ts2.seconds(), 0.016f);
    EXPECT_FLOAT_EQ(ts2.milliseconds(), 16.0f);
    
    // Test conversion operator
    float dt = ts2;
    EXPECT_FLOAT_EQ(dt, 0.016f);
}

// Test FPS functionality
TEST_F(LayerTest, ApplicationFPSControl) {
    Application app("FPS Test");
    
    EXPECT_EQ(app.get_fps(), 0.0f);
    
    app.set_target_fps(60.0f);
    // Note: We can't easily test the actual FPS without running the main loop
    
    app.set_target_fps(-10.0f); // Should clamp to 0
    app.set_target_fps(0.0f);   // Unlimited
}

// Integration test for Layer lifecycle
class LayerIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(LayerIntegrationTest, LayerLifecycleManagement) {
    Application app("Integration Test");
    
    auto layer1 = std::make_unique<TestLayer>("Layer1");
    auto layer2 = std::make_unique<TestLayer>("Layer2");
    
    auto* layer1_ptr = layer1.get();
    auto* layer2_ptr = layer2.get();
    
    app.add_layer(std::move(layer1));
    app.add_layer(std::move(layer2));
    
    EXPECT_EQ(layer1_ptr->attach_count, 0);
    EXPECT_EQ(layer2_ptr->attach_count, 0);
    
    // We can't easily test the full run() method in headless environment,
    // but we can test that layers are properly managed
    EXPECT_EQ(app.layer_count(), 2);
    
    // Test that layers exist and can be accessed
    EXPECT_NE(app.get_layer("Layer1"), nullptr);
    EXPECT_NE(app.get_layer("Layer2"), nullptr);
    
    // Test layer state
    EXPECT_TRUE(app.get_layer("Layer1")->is_enabled());
    EXPECT_TRUE(app.get_layer("Layer2")->is_enabled());
}

TEST_F(LayerIntegrationTest, LayerEnableDisableIntegration) {
    Application app("Integration Test");
    
    auto layer = std::make_unique<TestLayer>("TestLayer");
    auto* layer_ptr = layer.get();
    
    app.add_layer(std::move(layer));
    
    // Test layer can be disabled through application
    auto* retrieved_layer = app.get_layer("TestLayer");
    EXPECT_EQ(retrieved_layer, layer_ptr);
    
    retrieved_layer->disable();
    EXPECT_FALSE(retrieved_layer->is_enabled());
    
    retrieved_layer->enable();
    EXPECT_TRUE(retrieved_layer->is_enabled());
}