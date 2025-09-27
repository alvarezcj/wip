# Event System Library

A strongly-typed, thread-safe event system for C++ applications. This library provides type-safe event dispatching, priority-based processing, and automatic resource management.

## Quick Start

### Include Headers

```cpp
#include <event_dispatcher.h>  // Core event system
#include <common_events.h>     // Pre-defined event types (optional)
```

### Basic Usage

```cpp
#include <event_dispatcher.h>
#include <iostream>

using namespace wip::utils::event;

// Define a custom event
class ButtonClickEvent : public Event {
public:
    explicit ButtonClickEvent(std::string button_name)
        : button_name_(std::move(button_name)) {}
    
    const std::string& button_name() const { return button_name_; }

private:
    std::string button_name_;
};

int main() {
    // Create an event dispatcher
    EventDispatcher dispatcher;
    
    // Subscribe to events
    auto handle = dispatcher.subscribe<ButtonClickEvent>(
        [](const ButtonClickEvent& event) {
            std::cout << "Button clicked: " << event.button_name() << std::endl;
        }
    );
    
    // Dispatch events
    dispatcher.dispatch(ButtonClickEvent("OK"));
    dispatcher.dispatch(ButtonClickEvent("Cancel"));
    
    // handle automatically unsubscribes when it goes out of scope
    return 0;
}
```

### Advanced Features

#### Priority-based Processing
```cpp
// High priority handlers run first
auto high_priority = dispatcher.subscribe<MyEvent>(
    [](const MyEvent& e) { /* handle critical events first */ },
    Priority::High
);

auto normal_priority = dispatcher.subscribe<MyEvent>(
    [](const MyEvent& e) { /* handle normal events */ },
    Priority::Normal
);
```

#### Event Filtering
```cpp
// Only handle events that meet certain criteria
auto filtered = dispatcher.subscribe<PlayerEvent>(
    [](const PlayerEvent& e) { /* handle */ },
    Priority::Normal,
    [](const PlayerEvent& e) { return e.player_id() == 123; }  // filter
);
```

#### Event Consumption
```cpp
// Consume events to prevent further processing
dispatcher.subscribe<SystemEvent>(
    [](const SystemEvent& e) {
        if (e.type() == SystemEvent::Type::Shutdown) {
            // Handle shutdown
            e.consume(); // Stop further handlers from processing this event
        }
    },
    Priority::High
);
```

#### Thread-safe Operations
```cpp
// Safe to use from multiple threads
std::thread producer([&dispatcher]() {
    for (int i = 0; i < 100; ++i) {
        dispatcher.dispatch(DataEvent("counter", i));
    }
});

std::thread consumer([&dispatcher]() {
    auto handle = dispatcher.subscribe<DataEvent>(
        [](const DataEvent& e) {
            std::cout << "Received: " << e.key() << " = "
                      << e.data<int>() << std::endl;
        }
    );
    std::this_thread::sleep_for(std::chrono::seconds(1));
});

producer.join();
consumer.join();
```

#### Asynchronous Dispatch
```cpp
// Dispatch events asynchronously (non-blocking)
dispatcher.dispatch_async(MyEvent("background_task"));
```

#### Scoped Subscriptions (RAII)
```cpp
{
    auto scoped = dispatcher.subscribe_scoped<MyEvent>(
        [](const MyEvent& e) { /* handler */ }
    );
    
    dispatcher.dispatch(MyEvent("test"));
    // scoped automatically unsubscribes when it goes out of scope
}
```

#### Global Dispatcher
```cpp
// Use the global singleton dispatcher for convenience
auto& global = EventDispatcher::global_dispatcher();
global.subscribe<MyEvent>([](const MyEvent& e) { /* handle */ });
global.dispatch(MyEvent("global_event"));
```

## Common Event Types

The library includes pre-defined event types for common scenarios:

```cpp
#include <common_events.h>

// Simple message events
dispatcher.dispatch(MessageEvent("System initialized"));

// Data events with arbitrary payloads
dispatcher.dispatch(DataEvent("user_count", 42));

// Property change notifications
dispatcher.dispatch(PropertyChangeEvent("status", 
    std::string("offline"), std::string("online")));

// System lifecycle events
dispatcher.dispatch(SystemEvent(SystemEvent::Type::Startup, "App started"));

// GUI input events
dispatcher.dispatch(input::KeyEvent(
    input::KeyEvent::Type::Press, 'A', 0));
    
dispatcher.dispatch(input::MouseEvent(
    input::MouseEvent::Type::Press, 100.0, 200.0, 1));

// Network events
dispatcher.dispatch(network::ConnectionEvent(
    network::ConnectionEvent::Type::Connected, "server:8080"));
```

## Building

This library uses CMake and requires C++17:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make wip_utils_event
```

### Link in Your CMakeLists.txt

```cmake
find_package(wip_utils_event REQUIRED)
target_link_libraries(your_target PRIVATE wip::utils::event)
```

## Features

- **Type Safety**: Compile-time type checking prevents runtime errors
- **Thread Safety**: All operations are thread-safe with mutex protection
- **Priority Processing**: High-priority events are processed first
- **Event Filtering**: Subscribe only to events matching specific criteria
- **Event Consumption**: Prevent further processing of handled events
- **RAII Management**: Automatic subscription cleanup with scoped subscriptions
- **Asynchronous Dispatch**: Non-blocking event dispatch for performance
- **Global Access**: Singleton dispatcher for application-wide event handling
- **Performance**: Optimized for high-frequency event processing
- **Extensible**: Easy to create custom event types

## Thread Safety

All `EventDispatcher` operations are thread-safe:
- Multiple threads can subscribe/unsubscribe simultaneously
- Events can be dispatched from any thread
- Handlers are executed in the dispatching thread
- No data races or undefined behavior

## Performance Considerations

- Handlers are called synchronously by default
- Use `dispatch_async()` for non-blocking dispatch
- Event filtering happens at subscription time for efficiency
- Unsubscription is O(1) with subscription handles
- Memory usage is proportional to active subscriptions

## Examples

See `apps/event_system_demo/` for comprehensive examples including:
- Game event systems (players, items, scores)
- GUI event handling (keyboard, mouse, windows)
- Network event processing (connections, data transfer)

## Testing

Run the test suite:
```bash
./build/bin/test_wip_utils_event
```

Tests cover:
- Basic functionality and type safety
- Thread safety and concurrent access
- Performance under high load
- Edge cases and error conditions