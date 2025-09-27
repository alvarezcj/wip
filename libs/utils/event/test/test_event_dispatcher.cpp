#include <gtest/gtest.h>
#include <event_dispatcher.h>
#include <common_events.h>
#include <chrono>
#include <thread>

using namespace wip::utils::event;

// Test event classes
class TestEvent : public Event {
public:
    explicit TestEvent(int value) : value_(value) {}
    int value() const { return value_; }
private:
    int value_;
};

class AnotherTestEvent : public Event {
public:
    explicit AnotherTestEvent(std::string message) : message_(std::move(message)) {}
    const std::string& message() const { return message_; }
private:
    std::string message_;
};

class EventDispatcherTest : public ::testing::Test {
protected:
    void SetUp() override {
        dispatcher_ = std::make_unique<EventDispatcher>();
    }
    
    void TearDown() override {
        dispatcher_.reset();
    }
    
    std::unique_ptr<EventDispatcher> dispatcher_;
};

TEST_F(EventDispatcherTest, BasicSubscriptionAndDispatch) {
    int received_value = 0;
    
    auto handle = dispatcher_->subscribe<TestEvent>([&received_value](const TestEvent& e) {
        received_value = e.value();
    });
    
    EXPECT_TRUE(handle.is_valid());
    EXPECT_EQ(dispatcher_->subscription_count<TestEvent>(), 1);
    
    dispatcher_->dispatch(TestEvent{42});
    EXPECT_EQ(received_value, 42);
}

TEST_F(EventDispatcherTest, MultipleHandlersForSameEvent) {
    std::vector<int> received_values;
    
    auto handle1 = dispatcher_->subscribe<TestEvent>([&received_values](const TestEvent& e) {
        received_values.push_back(e.value() * 2);
    });
    
    auto handle2 = dispatcher_->subscribe<TestEvent>([&received_values](const TestEvent& e) {
        received_values.push_back(e.value() * 3);
    });
    
    EXPECT_EQ(dispatcher_->subscription_count<TestEvent>(), 2);
    
    dispatcher_->dispatch(TestEvent{10});
    
    EXPECT_EQ(received_values.size(), 2);
    // Note: Order depends on priority and registration order
    EXPECT_TRUE(std::find(received_values.begin(), received_values.end(), 20) != received_values.end());
    EXPECT_TRUE(std::find(received_values.begin(), received_values.end(), 30) != received_values.end());
}

TEST_F(EventDispatcherTest, DifferentEventTypes) {
    int test_value = 0;
    std::string test_message;
    
    dispatcher_->subscribe<TestEvent>([&test_value](const TestEvent& e) {
        test_value = e.value();
    });
    
    dispatcher_->subscribe<AnotherTestEvent>([&test_message](const AnotherTestEvent& e) {
        test_message = e.message();
    });
    
    EXPECT_EQ(dispatcher_->subscription_count<TestEvent>(), 1);
    EXPECT_EQ(dispatcher_->subscription_count<AnotherTestEvent>(), 1);
    
    dispatcher_->dispatch(TestEvent{123});
    dispatcher_->dispatch(AnotherTestEvent{"Hello"});
    
    EXPECT_EQ(test_value, 123);
    EXPECT_EQ(test_message, "Hello");
}

TEST_F(EventDispatcherTest, EventPriorities) {
    std::vector<int> execution_order;
    
    // Subscribe with different priorities
    dispatcher_->subscribe<TestEvent>([&execution_order](const TestEvent&) {
        execution_order.push_back(1);
    }, Priority::Low);
    
    dispatcher_->subscribe<TestEvent>([&execution_order](const TestEvent&) {
        execution_order.push_back(2);
    }, Priority::High);
    
    dispatcher_->subscribe<TestEvent>([&execution_order](const TestEvent&) {
        execution_order.push_back(3);
    }, Priority::Normal);
    
    dispatcher_->dispatch(TestEvent{1});
    
    // Should execute in priority order: High, Normal, Low
    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 2); // High priority
    EXPECT_EQ(execution_order[1], 3); // Normal priority
    EXPECT_EQ(execution_order[2], 1); // Low priority
}

TEST_F(EventDispatcherTest, EventConsumption) {
    std::vector<int> handled_values;
    
    // First handler consumes the event
    dispatcher_->subscribe<TestEvent>([&handled_values](const TestEvent& e) {
        handled_values.push_back(1);
        const_cast<TestEvent&>(e).consume();
    }, Priority::High);
    
    // Second handler should not be called
    dispatcher_->subscribe<TestEvent>([&handled_values](const TestEvent&) {
        handled_values.push_back(2);
    }, Priority::Low);
    
    dispatcher_->dispatch(TestEvent{1});
    
    ASSERT_EQ(handled_values.size(), 1);
    EXPECT_EQ(handled_values[0], 1);
}

TEST_F(EventDispatcherTest, EventFilter) {
    std::vector<int> received_values;
    
    // Only handle even values
    dispatcher_->subscribe<TestEvent>(
        [&received_values](const TestEvent& e) {
            received_values.push_back(e.value());
        },
        Priority::Normal,
        [](const TestEvent& e) { return e.value() % 2 == 0; }
    );
    
    dispatcher_->dispatch(TestEvent{1});  // Odd - should be filtered
    dispatcher_->dispatch(TestEvent{2});  // Even - should be handled
    dispatcher_->dispatch(TestEvent{3});  // Odd - should be filtered
    dispatcher_->dispatch(TestEvent{4});  // Even - should be handled
    
    ASSERT_EQ(received_values.size(), 2);
    EXPECT_EQ(received_values[0], 2);
    EXPECT_EQ(received_values[1], 4);
}

TEST_F(EventDispatcherTest, Unsubscription) {
    int call_count = 0;
    
    auto handle = dispatcher_->subscribe<TestEvent>([&call_count](const TestEvent&) {
        call_count++;
    });
    
    EXPECT_EQ(dispatcher_->subscription_count<TestEvent>(), 1);
    
    dispatcher_->dispatch(TestEvent{1});
    EXPECT_EQ(call_count, 1);
    
    bool unsubscribed = dispatcher_->unsubscribe(handle);
    EXPECT_TRUE(unsubscribed);
    EXPECT_EQ(dispatcher_->subscription_count<TestEvent>(), 0);
    
    dispatcher_->dispatch(TestEvent{2});
    EXPECT_EQ(call_count, 1); // Should not have increased
    
    // Try to unsubscribe again
    bool unsubscribed_again = dispatcher_->unsubscribe(handle);
    EXPECT_FALSE(unsubscribed_again);
}

TEST_F(EventDispatcherTest, ScopedSubscription) {
    int call_count = 0;
    
    {
        auto scoped_sub = dispatcher_->subscribe_scoped<TestEvent>([&call_count](const TestEvent&) {
            call_count++;
        });
        
        EXPECT_EQ(dispatcher_->subscription_count<TestEvent>(), 1);
        
        dispatcher_->dispatch(TestEvent{1});
        EXPECT_EQ(call_count, 1);
        
    } // scoped_sub goes out of scope here
    
    EXPECT_EQ(dispatcher_->subscription_count<TestEvent>(), 0);
    
    dispatcher_->dispatch(TestEvent{2});
    EXPECT_EQ(call_count, 1); // Should not have increased
}

TEST_F(EventDispatcherTest, PropagationControl) {
    std::vector<int> execution_order;
    
    // First handler stops propagation
    dispatcher_->subscribe_with_propagation<TestEvent>(
        [&execution_order](const TestEvent&) {
            execution_order.push_back(1);
            return false; // Stop propagation
        },
        Priority::High
    );
    
    // Second handler should not be called
    dispatcher_->subscribe<TestEvent>([&execution_order](const TestEvent&) {
        execution_order.push_back(2);
    }, Priority::Low);
    
    dispatcher_->dispatch(TestEvent{1});
    
    ASSERT_EQ(execution_order.size(), 1);
    EXPECT_EQ(execution_order[0], 1);
}

TEST_F(EventDispatcherTest, AsyncDispatch) {
    std::atomic<int> received_value{0};
    
    dispatcher_->subscribe<TestEvent>([&received_value](const TestEvent& e) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        received_value = e.value();
    });
    
    auto future = dispatcher_->dispatch_async(TestEvent{99});
    
    // Should be able to do other work while event is being processed
    EXPECT_EQ(received_value.load(), 0); // Not processed yet
    
    // Wait for completion
    size_t handlers_called = future.get();
    EXPECT_EQ(handlers_called, 1);
    EXPECT_EQ(received_value.load(), 99);
}

TEST_F(EventDispatcherTest, ClearSubscriptions) {
    dispatcher_->subscribe<TestEvent>([](const TestEvent&) {});
    dispatcher_->subscribe<TestEvent>([](const TestEvent&) {});
    dispatcher_->subscribe<AnotherTestEvent>([](const AnotherTestEvent&) {});
    
    EXPECT_EQ(dispatcher_->subscription_count<TestEvent>(), 2);
    EXPECT_EQ(dispatcher_->subscription_count<AnotherTestEvent>(), 1);
    EXPECT_EQ(dispatcher_->total_subscription_count(), 3);
    
    dispatcher_->clear_subscriptions<TestEvent>();
    
    EXPECT_EQ(dispatcher_->subscription_count<TestEvent>(), 0);
    EXPECT_EQ(dispatcher_->subscription_count<AnotherTestEvent>(), 1);
    EXPECT_EQ(dispatcher_->total_subscription_count(), 1);
    
    dispatcher_->clear_all_subscriptions();
    
    EXPECT_EQ(dispatcher_->subscription_count<AnotherTestEvent>(), 0);
    EXPECT_EQ(dispatcher_->total_subscription_count(), 0);
}

TEST_F(EventDispatcherTest, EventTimestamp) {
    auto start_time = std::chrono::steady_clock::now();
    
    std::chrono::steady_clock::time_point event_timestamp;
    
    dispatcher_->subscribe<TestEvent>([&event_timestamp](const TestEvent& e) {
        event_timestamp = e.timestamp();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    TestEvent event{1};
    auto dispatch_time = std::chrono::steady_clock::now();
    
    dispatcher_->dispatch(event);
    
    // Event timestamp should be between start_time and dispatch_time
    EXPECT_GE(event_timestamp, start_time);
    EXPECT_LE(event_timestamp, dispatch_time);
}

TEST(GlobalDispatcherTest, GlobalDispatcherSingleton) {
    auto& dispatcher1 = global_dispatcher();
    auto& dispatcher2 = global_dispatcher();
    
    EXPECT_EQ(&dispatcher1, &dispatcher2);
}

TEST(GlobalDispatcherTest, GlobalDispatcherPersistence) {
    int call_count = 0;
    
    // Subscribe using global dispatcher
    auto handle = global_dispatcher().subscribe<TestEvent>([&call_count](const TestEvent&) {
        call_count++;
    });
    
    // Access global dispatcher again and dispatch
    global_dispatcher().dispatch(TestEvent{1});
    
    EXPECT_EQ(call_count, 1);
    
    // Clean up
    global_dispatcher().unsubscribe(handle);
}