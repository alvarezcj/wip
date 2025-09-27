#pragma once

#include "event.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <future>

namespace wip::utils::event {

/**
 * @brief Unique identifier for event subscriptions.
 * 
 * This handle can be used to unsubscribe from events.
 */
class SubscriptionHandle {
public:
    using HandleType = uint64_t;
    
    SubscriptionHandle() : id_(0) {}
    explicit SubscriptionHandle(HandleType id) : id_(id) {}
    
    HandleType id() const noexcept { return id_; }
    bool is_valid() const noexcept { return id_ != 0; }
    
    bool operator==(const SubscriptionHandle& other) const noexcept {
        return id_ == other.id_;
    }
    
    bool operator!=(const SubscriptionHandle& other) const noexcept {
        return !(*this == other);
    }

private:
    HandleType id_;
};

/**
 * @brief RAII wrapper for automatic event unsubscription.
 * 
 * This class ensures that subscriptions are automatically removed when
 * the scoped subscription goes out of scope.
 */
class ScopedSubscription {
public:
    ScopedSubscription() = default;
    
    ScopedSubscription(std::function<void()> unsubscribe_func)
        : unsubscribe_func_(std::move(unsubscribe_func)) {}
    
    ~ScopedSubscription() {
        if (unsubscribe_func_) {
            unsubscribe_func_();
        }
    }
    
    // Move-only type
    ScopedSubscription(const ScopedSubscription&) = delete;
    ScopedSubscription& operator=(const ScopedSubscription&) = delete;
    
    ScopedSubscription(ScopedSubscription&& other) noexcept
        : unsubscribe_func_(std::move(other.unsubscribe_func_)) {
        other.unsubscribe_func_ = nullptr;
    }
    
    ScopedSubscription& operator=(ScopedSubscription&& other) noexcept {
        if (this != &other) {
            if (unsubscribe_func_) {
                unsubscribe_func_();
            }
            unsubscribe_func_ = std::move(other.unsubscribe_func_);
            other.unsubscribe_func_ = nullptr;
        }
        return *this;
    }

private:
    std::function<void()> unsubscribe_func_;
};

/**
 * @brief Thread-safe, strongly-typed event dispatcher.
 * 
 * This class provides a complete event system with the following features:
 * - Strong type safety using templates
 * - Thread-safe operation
 * - Priority-based event handling
 * - Synchronous and asynchronous event dispatch
 * - Event filtering
 * - Automatic subscription management
 */
class EventDispatcher {
private:
    struct HandlerInfo {
        std::function<void(const Event&)> handler;
        Priority priority;
        std::function<bool(const Event&)> filter;
        SubscriptionHandle::HandleType id;
        
        bool operator<(const HandlerInfo& other) const {
            // Reverse comparison for priority queue (higher priority first)
            return static_cast<int>(priority) < static_cast<int>(other.priority);
        }
    };
    
    using HandlerList = std::vector<HandlerInfo>;

public:
    EventDispatcher() : next_subscription_id_(1) {}
    ~EventDispatcher() = default;
    
    // Non-copyable, non-movable
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;
    EventDispatcher(EventDispatcher&&) = delete;
    EventDispatcher& operator=(EventDispatcher&&) = delete;
    
    /**
     * @brief Subscribe to events of a specific type.
     * 
     * @tparam EventType The type of event to subscribe to (must inherit from Event)
     * @param handler The function to call when the event is dispatched
     * @param priority Priority level for this handler (default: Normal)
     * @param filter Optional filter function to conditionally handle events
     * @return Handle that can be used to unsubscribe
     */
    template<typename EventType>
    SubscriptionHandle subscribe(EventHandler<EventType> handler, 
                               Priority priority = Priority::Normal,
                               std::function<bool(const EventType&)> filter = nullptr) {
        static_assert(is_event_v<EventType>, "EventType must inherit from Event");
        
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        auto subscription_id = next_subscription_id_++;
        auto type_index = std::type_index(typeid(EventType));
        
        // Wrap the typed handler in a generic handler
        auto generic_handler = [handler](const Event& event) {
            const auto& typed_event = static_cast<const EventType&>(event);
            handler(typed_event);
        };
        
        // Wrap the typed filter in a generic filter if provided
        std::function<bool(const Event&)> generic_filter = nullptr;
        if (filter) {
            generic_filter = [filter](const Event& event) -> bool {
                const auto& typed_event = static_cast<const EventType&>(event);
                return filter(typed_event);
            };
        }
        
        handlers_[type_index].push_back({
            std::move(generic_handler),
            priority,
            std::move(generic_filter),
            subscription_id
        });
        
        // Sort handlers by priority (highest first)
        std::sort(handlers_[type_index].begin(), handlers_[type_index].end(),
                 [](const HandlerInfo& a, const HandlerInfo& b) {
                     return static_cast<int>(a.priority) > static_cast<int>(b.priority);
                 });
        
        return SubscriptionHandle(subscription_id);
    }
    
    /**
     * @brief Subscribe to events with propagation control.
     * 
     * @tparam EventType The type of event to subscribe to
     * @param handler Handler that returns true to continue propagation, false to stop
     * @param priority Priority level for this handler (default: Normal)
     * @param filter Optional filter function
     * @return Handle that can be used to unsubscribe
     */
    template<typename EventType>
    SubscriptionHandle subscribe_with_propagation(
        EventHandlerWithPropagation<EventType> handler,
        Priority priority = Priority::Normal,
        std::function<bool(const EventType&)> filter = nullptr) {
        
        // Convert propagation handler to regular handler that marks consumption
        auto regular_handler = [handler](const EventType& event) {
            bool should_continue = handler(event);
            if (!should_continue) {
                const_cast<EventType&>(event).consume();
            }
        };
        
        return subscribe<EventType>(std::move(regular_handler), priority, std::move(filter));
    }
    
    /**
     * @brief Create a scoped subscription that automatically unsubscribes.
     * 
     * @tparam EventType The type of event to subscribe to
     * @param handler The event handler function
     * @param priority Priority level (default: Normal)
     * @param filter Optional filter function
     * @return Scoped subscription that unsubscribes when destroyed
     */
    template<typename EventType>
    ScopedSubscription subscribe_scoped(EventHandler<EventType> handler,
                                      Priority priority = Priority::Normal,
                                      std::function<bool(const EventType&)> filter = nullptr) {
        auto handle = subscribe<EventType>(std::move(handler), priority, std::move(filter));
        
        return ScopedSubscription([this, handle]() {
            unsubscribe(handle);
        });
    }
    
    /**
     * @brief Unsubscribe from events using a subscription handle.
     * 
     * @param handle The subscription handle returned from subscribe()
     * @return True if the subscription was found and removed, false otherwise
     */
    bool unsubscribe(SubscriptionHandle handle);
    
    /**
     * @brief Dispatch an event synchronously to all registered handlers.
     * 
     * @tparam EventType The type of event to dispatch
     * @param event The event to dispatch
     * @return Number of handlers that processed the event
     */
    template<typename EventType>
    size_t dispatch(const EventType& event) {
        static_assert(is_event_v<EventType>, "EventType must inherit from Event");
        
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        auto type_index = std::type_index(typeid(EventType));
        auto it = handlers_.find(type_index);
        
        if (it == handlers_.end()) {
            return 0;
        }
        
        size_t handlers_called = 0;
        for (const auto& handler_info : it->second) {
            if (event.is_consumed()) {
                break;  // Stop processing if event was consumed
            }
            
            // Apply filter if present
            if (handler_info.filter && !handler_info.filter(event)) {
                continue;
            }
            
            try {
                handler_info.handler(event);
                ++handlers_called;
            } catch (const std::exception& e) {
                // Log error but continue processing other handlers
                // In a real implementation, you might want to use your logging library
                (void)e; // Suppress unused variable warning
            }
        }
        
        return handlers_called;
    }
    
    /**
     * @brief Dispatch an event asynchronously.
     * 
     * @tparam EventType The type of event to dispatch
     * @param event The event to dispatch (will be copied)
     * @return Future that resolves to the number of handlers called
     */
    template<typename EventType>
    std::future<size_t> dispatch_async(EventType event) {
        static_assert(is_event_v<EventType>, "EventType must inherit from Event");
        
        return std::async(std::launch::async, [this, event = std::move(event)]() {
            return dispatch(event);
        });
    }
    
    /**
     * @brief Get the number of active subscriptions for a specific event type.
     * 
     * @tparam EventType The event type to check
     * @return Number of active subscriptions
     */
    template<typename EventType>
    size_t subscription_count() const {
        static_assert(is_event_v<EventType>, "EventType must inherit from Event");
        
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        auto type_index = std::type_index(typeid(EventType));
        auto it = handlers_.find(type_index);
        
        return it != handlers_.end() ? it->second.size() : 0;
    }
    
    /**
     * @brief Get the total number of active subscriptions across all event types.
     * 
     * @return Total number of active subscriptions
     */
    size_t total_subscription_count() const;
    
    /**
     * @brief Clear all subscriptions for a specific event type.
     * 
     * @tparam EventType The event type to clear subscriptions for
     */
    template<typename EventType>
    void clear_subscriptions() {
        static_assert(is_event_v<EventType>, "EventType must inherit from Event");
        
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        auto type_index = std::type_index(typeid(EventType));
        handlers_.erase(type_index);
    }
    
    /**
     * @brief Clear all subscriptions for all event types.
     */
    void clear_all_subscriptions();

private:
    mutable std::mutex handlers_mutex_;
    std::unordered_map<std::type_index, HandlerList> handlers_;
    std::atomic<SubscriptionHandle::HandleType> next_subscription_id_;
};

/**
 * @brief Get a global event dispatcher instance.
 * 
 * This provides a convenient way to access a shared event dispatcher
 * without having to manage the lifetime manually. The dispatcher is
 * created on first access and persists for the lifetime of the program.
 * 
 * @return Reference to the global event dispatcher
 * 
 * @note This is thread-safe and the returned reference is valid for the
 *       entire program lifetime.
 */
EventDispatcher& global_dispatcher();

} // namespace wip::utils::event