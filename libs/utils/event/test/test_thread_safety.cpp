#include <gtest/gtest.h>
#include <event_dispatcher.h>
#include <common_events.h>
#include <thread>
#include <vector>
#include <atomic>
#include <future>
#include <random>

using namespace wip::utils::event;

class ThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        dispatcher_ = std::make_unique<EventDispatcher>();
    }
    
    void TearDown() override {
        dispatcher_.reset();
    }
    
    std::unique_ptr<EventDispatcher> dispatcher_;
};

class CounterEvent : public Event {
public:
    explicit CounterEvent(int value) : value_(value) {}
    int value() const { return value_; }
private:
    int value_;
};

TEST_F(ThreadSafetyTest, ConcurrentSubscriptions) {
    const int num_threads = 10;
    const int subscriptions_per_thread = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> subscription_count{0};
    
    // Launch threads that each create multiple subscriptions
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < subscriptions_per_thread; ++j) {
                dispatcher_->subscribe<CounterEvent>([&subscription_count](const CounterEvent&) {
                    subscription_count++;
                });
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(dispatcher_->subscription_count<CounterEvent>(), num_threads * subscriptions_per_thread);
}

TEST_F(ThreadSafetyTest, ConcurrentDispatchAndSubscription) {
    std::atomic<int> event_count{0};
    std::atomic<bool> stop_dispatching{false};
    
    // Thread that continuously dispatches events
    std::thread dispatcher_thread([&]() {
        int counter = 0;
        while (!stop_dispatching.load()) {
            dispatcher_->dispatch(CounterEvent{counter++});
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    });
    
    // Threads that subscribe and unsubscribe
    std::vector<std::thread> subscriber_threads;
    const int num_subscriber_threads = 5;
    
    for (int i = 0; i < num_subscriber_threads; ++i) {
        subscriber_threads.emplace_back([&]() {
            for (int j = 0; j < 50; ++j) {
                auto handle = dispatcher_->subscribe<CounterEvent>([&event_count](const CounterEvent&) {
                    event_count++;
                });
                
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                
                dispatcher_->unsubscribe(handle);
            }
        });
    }
    
    // Let it run for a while
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    stop_dispatching = true;
    dispatcher_thread.join();
    
    for (auto& t : subscriber_threads) {
        t.join();
    }
    
    // Should have received some events (exact count depends on timing)
    EXPECT_GT(event_count.load(), 0);
}

TEST_F(ThreadSafetyTest, ConcurrentEventDispatch) {
    const int num_threads = 8;
    const int events_per_thread = 1000;
    
    std::atomic<int> total_events_received{0};
    std::atomic<int> sum_of_values{0};
    
    // Single handler that counts events and sums values
    dispatcher_->subscribe<CounterEvent>([&](const CounterEvent& e) {
        total_events_received++;
        sum_of_values += e.value();
    });
    
    std::vector<std::thread> threads;
    
    // Launch threads that dispatch events concurrently
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            int base_value = i * events_per_thread;
            for (int j = 0; j < events_per_thread; ++j) {
                dispatcher_->dispatch(CounterEvent{base_value + j});
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(total_events_received.load(), num_threads * events_per_thread);
    
    // Calculate expected sum: sum of arithmetic sequence
    int expected_sum = 0;
    for (int i = 0; i < num_threads; ++i) {
        int base = i * events_per_thread;
        for (int j = 0; j < events_per_thread; ++j) {
            expected_sum += base + j;
        }
    }
    
    EXPECT_EQ(sum_of_values.load(), expected_sum);
}

TEST_F(ThreadSafetyTest, AsyncEventDispatch) {
    const int num_async_dispatches = 100;
    std::atomic<int> events_received{0};
    
    dispatcher_->subscribe<CounterEvent>([&events_received](const CounterEvent&) {
        events_received++;
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    });
    
    std::vector<std::future<size_t>> futures;
    
    // Launch multiple async dispatches
    for (int i = 0; i < num_async_dispatches; ++i) {
        futures.push_back(dispatcher_->dispatch_async(CounterEvent{i}));
    }
    
    // Wait for all async dispatches to complete
    size_t total_handlers_called = 0;
    for (auto& future : futures) {
        total_handlers_called += future.get();
    }
    
    EXPECT_EQ(events_received.load(), num_async_dispatches);
    EXPECT_EQ(total_handlers_called, num_async_dispatches);
}

TEST_F(ThreadSafetyTest, StressTestWithRandomOperations) {
    const int num_threads = 6;
    const int operations_per_thread = 500;
    
    std::atomic<int> events_received{0};
    std::atomic<int> subscriptions_created{0};
    std::atomic<int> subscriptions_removed{0};
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::mt19937 rng(i);  // Different seed per thread
            std::uniform_int_distribution<int> op_dist(0, 2);
            std::vector<SubscriptionHandle> handles;
            
            for (int j = 0; j < operations_per_thread; ++j) {
                int operation = op_dist(rng);
                
                switch (operation) {
                    case 0: // Subscribe
                    {
                        auto handle = dispatcher_->subscribe<CounterEvent>([&events_received](const CounterEvent&) {
                            events_received++;
                        });
                        handles.push_back(handle);
                        subscriptions_created++;
                        break;
                    }
                    case 1: // Unsubscribe
                    {
                        if (!handles.empty()) {
                            auto handle = handles.back();
                            handles.pop_back();
                            if (dispatcher_->unsubscribe(handle)) {
                                subscriptions_removed++;
                            }
                        }
                        break;
                    }
                    case 2: // Dispatch event
                    {
                        dispatcher_->dispatch(CounterEvent{j});
                        break;
                    }
                }
                
                // Small delay to increase chance of race conditions
                if (j % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
            
            // Clean up remaining handles
            for (auto handle : handles) {
                if (dispatcher_->unsubscribe(handle)) {
                    subscriptions_removed++;
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify consistency
    EXPECT_EQ(subscriptions_created.load(), subscriptions_removed.load());
    EXPECT_EQ(dispatcher_->total_subscription_count(), 0);
    
    // Should have received some events
    EXPECT_GE(events_received.load(), 0);
}

TEST_F(ThreadSafetyTest, ScopedSubscriptionsInMultipleThreads) {
    const int num_threads = 8;
    auto new_dispatcher = std::make_unique<EventDispatcher>();
    std::atomic<int> events_received{0};
    
    // Use a barrier to synchronize thread execution phases
    std::vector<std::thread> threads;
    std::atomic<int> subscriptions_active{0};
    std::atomic<int> events_dispatched_while_active{0};
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            // Create scoped subscription
            {
                auto scoped_sub = new_dispatcher->subscribe_scoped<CounterEvent>([&events_received](const CounterEvent&) {
                    events_received++;
                });
                
                subscriptions_active++;
                
                // Dispatch some events while subscription is active
                for (int j = 0; j < 10; ++j) {
                    new_dispatcher->dispatch(CounterEvent{i * 10 + j});
                    events_dispatched_while_active++;
                }
                
                // Give other threads time to create their subscriptions
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                
            } // scoped_sub goes out of scope here
            
            subscriptions_active--;
            
            // Wait for all subscriptions to be destroyed
            while (subscriptions_active.load() > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
            
            // Dispatch more events after all subscriptions are destroyed
            // These should not be received by anyone
            for (int j = 0; j < 5; ++j) {
                new_dispatcher->dispatch(CounterEvent{i * 10 + j + 100});
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    // Should have no active subscriptions
    EXPECT_EQ(new_dispatcher->subscription_count<CounterEvent>(), 0);
    
    // Should have received some events while subscriptions were active
    // The exact count depends on timing and thread interleaving, but should be:
    // - Greater than 0 (we dispatched events while subscriptions existed)
    // - Less than or equal to total_events_dispatched * max_concurrent_subscriptions
    EXPECT_GT(events_received.load(), 0);
    
    // We dispatched 80 events total while subscriptions were active
    // In the worst case, all 8 subscriptions could be active when all 80 events are dispatched
    // So maximum possible is 80 * 8 = 640, but due to timing it should be much less
    EXPECT_LE(events_received.load(), events_dispatched_while_active.load() * num_threads);
}