#include <gtest/gtest.h>
#include <window.h>
#include <event_dispatcher.h>
#include <atomic>
#include <vector>

using namespace wip::gui::window;
using namespace wip::gui::window::events;
using namespace wip::utils::event;

class WindowEventsTest : public ::testing::Test {
protected:
    void SetUp() override {
        event_dispatcher_ = std::make_unique<EventDispatcher>();
        window_event_count_ = 0;
        keyboard_event_count_ = 0;
        mouse_event_count_ = 0;
    }
    
    void TearDown() override {
        event_dispatcher_.reset();
    }
    
    // Helper function to check if we can create windows (not in CI/headless)
    bool can_create_window() {
        try {
            WindowConfig config;
            config.visible = false;
            Window test_window(config);
            return true;
        } catch (const std::runtime_error&) {
            return false;
        }
    }
    
    std::unique_ptr<EventDispatcher> event_dispatcher_;
    std::atomic<int> window_event_count_;
    std::atomic<int> keyboard_event_count_;
    std::atomic<int> mouse_event_count_;
};

TEST_F(WindowEventsTest, WindowEventTypes) {
    // Test that our window event types are properly defined
    WindowEvent close_event(WindowEvent::Type::Close);
    EXPECT_EQ(close_event.type(), WindowEvent::Type::Close);
    
    WindowEvent resize_event(WindowEvent::Type::Resize, 800, 600);
    EXPECT_EQ(resize_event.type(), WindowEvent::Type::Resize);
    EXPECT_EQ(resize_event.width(), 800);
    EXPECT_EQ(resize_event.height(), 600);
    
    WindowEvent move_event(WindowEvent::Type::Move, 0, 0, 100, 200);
    EXPECT_EQ(move_event.type(), WindowEvent::Type::Move);
    EXPECT_EQ(move_event.x(), 100);
    EXPECT_EQ(move_event.y(), 200);
}

TEST_F(WindowEventsTest, KeyboardEventTypes) {
    KeyboardEvent key_event(GLFW_KEY_A, 30, KeyboardEvent::Action::Press, GLFW_MOD_CONTROL);
    
    EXPECT_EQ(key_event.key(), GLFW_KEY_A);
    EXPECT_EQ(key_event.scancode(), 30);
    EXPECT_EQ(key_event.action(), KeyboardEvent::Action::Press);
    EXPECT_EQ(key_event.mods(), GLFW_MOD_CONTROL);
    
    EXPECT_FALSE(key_event.is_shift());
    EXPECT_TRUE(key_event.is_ctrl());
    EXPECT_FALSE(key_event.is_alt());
    EXPECT_FALSE(key_event.is_super());
}

TEST_F(WindowEventsTest, CharacterEventTypes) {
    CharacterEvent char_event('A');
    
    EXPECT_EQ(char_event.codepoint(), static_cast<unsigned int>('A'));
    EXPECT_EQ(char_event.as_char(), 'A');
}

TEST_F(WindowEventsTest, MouseButtonEventTypes) {
    MouseButtonEvent mouse_event(GLFW_MOUSE_BUTTON_LEFT, MouseButtonEvent::Action::Press, 0, 150.5, 200.7);
    
    EXPECT_EQ(mouse_event.button(), GLFW_MOUSE_BUTTON_LEFT);
    EXPECT_EQ(mouse_event.action(), MouseButtonEvent::Action::Press);
    EXPECT_EQ(mouse_event.mods(), 0);
    EXPECT_DOUBLE_EQ(mouse_event.x(), 150.5);
    EXPECT_DOUBLE_EQ(mouse_event.y(), 200.7);
    
    EXPECT_TRUE(mouse_event.is_left_button());
    EXPECT_FALSE(mouse_event.is_right_button());
    EXPECT_FALSE(mouse_event.is_middle_button());
}

TEST_F(WindowEventsTest, MouseMoveEventTypes) {
    MouseMoveEvent move_event(100.5, 200.7, 10.2, -5.3);
    
    EXPECT_DOUBLE_EQ(move_event.x(), 100.5);
    EXPECT_DOUBLE_EQ(move_event.y(), 200.7);
    EXPECT_DOUBLE_EQ(move_event.dx(), 10.2);
    EXPECT_DOUBLE_EQ(move_event.dy(), -5.3);
}

TEST_F(WindowEventsTest, MouseScrollEventTypes) {
    MouseScrollEvent scroll_event(1.5, -2.0);
    
    EXPECT_DOUBLE_EQ(scroll_event.x_offset(), 1.5);
    EXPECT_DOUBLE_EQ(scroll_event.y_offset(), -2.0);
}

TEST_F(WindowEventsTest, EventInheritance) {
    // Test that our events properly inherit from Event base class
    WindowEvent window_event(WindowEvent::Type::Close);
    KeyboardEvent key_event(GLFW_KEY_SPACE, 57, KeyboardEvent::Action::Press, 0);
    MouseButtonEvent mouse_event(GLFW_MOUSE_BUTTON_RIGHT, MouseButtonEvent::Action::Release, 0, 0, 0);
    
    // Should be able to treat them as base Event objects
    const Event* events[] = { &window_event, &key_event, &mouse_event };
    
    for (const auto* event : events) {
        EXPECT_NE(event, nullptr);
        // All events should have timestamps
        EXPECT_GT(event->timestamp().time_since_epoch().count(), 0);
    }
}

TEST_F(WindowEventsTest, EventSubscriptionAndDispatch) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    // Test that we can subscribe to window events
    auto window_handle = event_dispatcher_->subscribe<WindowEvent>([this](const WindowEvent&) {
        window_event_count_++;
    });
    
    auto key_handle = event_dispatcher_->subscribe<KeyboardEvent>([this](const KeyboardEvent&) {
        keyboard_event_count_++;
    });
    
    auto mouse_handle = event_dispatcher_->subscribe<MouseButtonEvent>([this](const MouseButtonEvent&) {
        mouse_event_count_++;
    });
    
    // Manually dispatch some events to test the event types work with dispatcher
    event_dispatcher_->dispatch(WindowEvent(WindowEvent::Type::Resize, 1024, 768));
    event_dispatcher_->dispatch(KeyboardEvent(GLFW_KEY_ENTER, 28, KeyboardEvent::Action::Press, 0));
    event_dispatcher_->dispatch(MouseButtonEvent(GLFW_MOUSE_BUTTON_LEFT, MouseButtonEvent::Action::Press, 0, 100, 200));
    
    EXPECT_EQ(window_event_count_.load(), 1);
    EXPECT_EQ(keyboard_event_count_.load(), 1);
    EXPECT_EQ(mouse_event_count_.load(), 1);
}

TEST_F(WindowEventsTest, WindowEventDispatcherIntegration) {
    if (!can_create_window()) {
        GTEST_SKIP() << "Cannot create windows in this environment";
    }
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    window.set_event_dispatcher(event_dispatcher_.get());
    
    // Subscribe to window events
    std::vector<WindowEvent> received_events;
    auto handle = event_dispatcher_->subscribe<WindowEvent>([&received_events](const WindowEvent& e) {
        received_events.push_back(e);
    });
    
    // Manually trigger a close event (this would normally come from GLFW)
    event_dispatcher_->dispatch(WindowEvent(WindowEvent::Type::Close));
    
    EXPECT_EQ(received_events.size(), 1);
    EXPECT_EQ(received_events[0].type(), WindowEvent::Type::Close);
    
    // Test resize event
    event_dispatcher_->dispatch(WindowEvent(WindowEvent::Type::Resize, 1280, 720));
    
    EXPECT_EQ(received_events.size(), 2);
    EXPECT_EQ(received_events[1].type(), WindowEvent::Type::Resize);
    EXPECT_EQ(received_events[1].width(), 1280);
    EXPECT_EQ(received_events[1].height(), 720);
}

TEST_F(WindowEventsTest, KeyboardEventModifiers) {
    // Test all modifier combinations
    KeyboardEvent shift_ctrl(GLFW_KEY_A, 30, KeyboardEvent::Action::Press, GLFW_MOD_SHIFT | GLFW_MOD_CONTROL);
    EXPECT_TRUE(shift_ctrl.is_shift());
    EXPECT_TRUE(shift_ctrl.is_ctrl());
    EXPECT_FALSE(shift_ctrl.is_alt());
    EXPECT_FALSE(shift_ctrl.is_super());
    
    KeyboardEvent alt_super(GLFW_KEY_B, 48, KeyboardEvent::Action::Press, GLFW_MOD_ALT | GLFW_MOD_SUPER);
    EXPECT_FALSE(alt_super.is_shift());
    EXPECT_FALSE(alt_super.is_ctrl());
    EXPECT_TRUE(alt_super.is_alt());
    EXPECT_TRUE(alt_super.is_super());
}

TEST_F(WindowEventsTest, MouseButtonHelpers) {
    // Test left button
    MouseButtonEvent left(GLFW_MOUSE_BUTTON_LEFT, MouseButtonEvent::Action::Press, 0, 0, 0);
    EXPECT_TRUE(left.is_left_button());
    EXPECT_FALSE(left.is_right_button());
    EXPECT_FALSE(left.is_middle_button());
    
    // Test right button
    MouseButtonEvent right(GLFW_MOUSE_BUTTON_RIGHT, MouseButtonEvent::Action::Press, 0, 0, 0);
    EXPECT_FALSE(right.is_left_button());
    EXPECT_TRUE(right.is_right_button());
    EXPECT_FALSE(right.is_middle_button());
    
    // Test middle button
    MouseButtonEvent middle(GLFW_MOUSE_BUTTON_MIDDLE, MouseButtonEvent::Action::Press, 0, 0, 0);
    EXPECT_FALSE(middle.is_left_button());
    EXPECT_FALSE(middle.is_right_button());
    EXPECT_TRUE(middle.is_middle_button());
}

TEST_F(WindowEventsTest, EventConsumption) {
    // Test that events can be consumed to prevent further processing
    int handler1_count = 0;
    int handler2_count = 0;
    
    // First handler consumes the event
    auto handle1 = event_dispatcher_->subscribe<WindowEvent>([&handler1_count](const WindowEvent& e) {
        handler1_count++;
        const_cast<WindowEvent&>(e).consume(); // Consume the event
    }, Priority::High);
    
    // Second handler should not receive consumed events
    auto handle2 = event_dispatcher_->subscribe<WindowEvent>([&handler2_count](const WindowEvent&) {
        handler2_count++;
    }, Priority::Normal);
    
    event_dispatcher_->dispatch(WindowEvent(WindowEvent::Type::Close));
    
    EXPECT_EQ(handler1_count, 1);
    EXPECT_EQ(handler2_count, 0); // Should not receive consumed event
}

TEST_F(WindowEventsTest, EventPriority) {
    std::vector<int> execution_order;
    
    // Subscribe with different priorities
    auto normal_handle = event_dispatcher_->subscribe<WindowEvent>([&execution_order](const WindowEvent&) {
        execution_order.push_back(1); // Normal priority
    }, Priority::Normal);
    
    auto high_handle = event_dispatcher_->subscribe<WindowEvent>([&execution_order](const WindowEvent&) {
        execution_order.push_back(2); // High priority
    }, Priority::High);
    
    auto low_handle = event_dispatcher_->subscribe<WindowEvent>([&execution_order](const WindowEvent&) {
        execution_order.push_back(3); // Low priority
    }, Priority::Low);
    
    event_dispatcher_->dispatch(WindowEvent(WindowEvent::Type::Focus));
    
    // Should execute in priority order: High, Normal, Low
    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 2); // High priority first
    EXPECT_EQ(execution_order[1], 1); // Normal priority second
    EXPECT_EQ(execution_order[2], 3); // Low priority last
}