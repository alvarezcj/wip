#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <chrono>

namespace wip::utils::event {

/**
 * @brief Base class for all events in the system.
 * 
 * This class provides a common interface for all events and serves as a marker
 * to ensure type safety in the event system. All user-defined events should
 * inherit from this class.
 */
class Event {
public:
    virtual ~Event() = default;
    
    /**
     * @brief Get the timestamp when the event was created.
     * @return Time point of event creation
     */
    auto timestamp() const noexcept -> std::chrono::steady_clock::time_point {
        return timestamp_;
    }
    
    /**
     * @brief Check if the event has been consumed (handled).
     * @return True if the event was consumed by a handler
     */
    bool is_consumed() const noexcept {
        return consumed_;
    }
    
    /**
     * @brief Mark the event as consumed to prevent further processing.
     */
    void consume() noexcept {
        consumed_ = true;
    }

protected:
    Event() : timestamp_(std::chrono::steady_clock::now()), consumed_(false) {}

private:
    std::chrono::steady_clock::time_point timestamp_;
    bool consumed_;
};

/**
 * @brief Type trait to check if a type is a valid event type.
 * 
 * Valid event types must inherit from Event.
 */
template<typename T>
struct is_event : std::is_base_of<Event, T> {};

template<typename T>
inline constexpr bool is_event_v = is_event<T>::value;

/**
 * @brief Priority levels for event handling.
 * 
 * Higher priority events are processed before lower priority ones.
 */
enum class Priority : int {
    Lowest = 0,
    Low = 25,
    Normal = 50,
    High = 75,
    Highest = 100,
    Critical = 1000
};

/**
 * @brief Type alias for event handler functions.
 * 
 * Event handlers are functions that take a const reference to the event type
 * and optionally return whether the event should continue propagating.
 */
template<typename EventType>
using EventHandler = std::function<void(const EventType&)>;

/**
 * @brief Type alias for event handler functions that can stop propagation.
 * 
 * These handlers return true to continue propagation, false to stop it.
 */
template<typename EventType>
using EventHandlerWithPropagation = std::function<bool(const EventType&)>;

} // namespace wip::utils::event