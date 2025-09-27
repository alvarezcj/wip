#include <gtest/gtest.h>
#include <common_events.h>
#include <event_dispatcher.h>

using namespace wip::utils::event;

TEST(CommonEventsTest, MessageEvent) {
    EventDispatcher dispatcher;
    std::string received_message;
    
    dispatcher.subscribe<MessageEvent>([&received_message](const MessageEvent& e) {
        received_message = e.message();
    });
    
    dispatcher.dispatch(MessageEvent{"Hello, World!"});
    
    EXPECT_EQ(received_message, "Hello, World!");
}

TEST(CommonEventsTest, DataEvent) {
    EventDispatcher dispatcher;
    int received_int = 0;
    std::string received_string;
    
    dispatcher.subscribe<DataEvent>([&received_int, &received_string](const DataEvent& e) {
        if (e.key() == "integer" && e.has_data<int>()) {
            received_int = e.data<int>();
        } else if (e.key() == "string" && e.has_data<std::string>()) {
            received_string = e.data<std::string>();
        }
    });
    
    dispatcher.dispatch(DataEvent{"integer", 42});
    dispatcher.dispatch(DataEvent{"string", std::string{"test"}});
    
    EXPECT_EQ(received_int, 42);
    EXPECT_EQ(received_string, "test");
}

TEST(CommonEventsTest, PropertyChangeEvent) {
    EventDispatcher dispatcher;
    std::string property_name;
    int old_val = 0, new_val = 0;
    
    dispatcher.subscribe<PropertyChangeEvent>([&](const PropertyChangeEvent& e) {
        property_name = e.property_name();
        old_val = e.old_value<int>();
        new_val = e.new_value<int>();
    });
    
    dispatcher.dispatch(PropertyChangeEvent{"age", 25, 26});
    
    EXPECT_EQ(property_name, "age");
    EXPECT_EQ(old_val, 25);
    EXPECT_EQ(new_val, 26);
}

TEST(CommonEventsTest, SystemEvent) {
    EventDispatcher dispatcher;
    SystemEvent::Type received_type;
    std::string received_message;
    
    dispatcher.subscribe<SystemEvent>([&](const SystemEvent& e) {
        received_type = e.type();
        received_message = e.message();
    });
    
    dispatcher.dispatch(SystemEvent{SystemEvent::Type::Error, "Something went wrong"});
    
    EXPECT_EQ(received_type, SystemEvent::Type::Error);
    EXPECT_EQ(received_message, "Something went wrong");
}

TEST(CommonEventsTest, KeyEvent) {
    EventDispatcher dispatcher;
    input::KeyEvent::Type received_type;
    int received_key = 0, received_modifiers = 0;
    
    dispatcher.subscribe<input::KeyEvent>([&](const input::KeyEvent& e) {
        received_type = e.type();
        received_key = e.key_code();
        received_modifiers = e.modifiers();
    });
    
    dispatcher.dispatch(input::KeyEvent{input::KeyEvent::Type::Press, 65, 1}); // 'A' key with Shift
    
    EXPECT_EQ(received_type, input::KeyEvent::Type::Press);
    EXPECT_EQ(received_key, 65);
    EXPECT_EQ(received_modifiers, 1);
}

TEST(CommonEventsTest, MouseEvent) {
    EventDispatcher dispatcher;
    input::MouseEvent::Type received_type;
    double received_x = 0, received_y = 0;
    int received_button = 0;
    
    dispatcher.subscribe<input::MouseEvent>([&](const input::MouseEvent& e) {
        received_type = e.type();
        received_x = e.x();
        received_y = e.y();
        received_button = e.button();
    });
    
    dispatcher.dispatch(input::MouseEvent{input::MouseEvent::Type::Press, 100.5, 200.7, 1});
    
    EXPECT_EQ(received_type, input::MouseEvent::Type::Press);
    EXPECT_DOUBLE_EQ(received_x, 100.5);
    EXPECT_DOUBLE_EQ(received_y, 200.7);
    EXPECT_EQ(received_button, 1);
}

TEST(CommonEventsTest, ConnectionEvent) {
    EventDispatcher dispatcher;
    network::ConnectionEvent::Type received_type;
    std::string received_endpoint, received_data;
    
    dispatcher.subscribe<network::ConnectionEvent>([&](const network::ConnectionEvent& e) {
        received_type = e.type();
        received_endpoint = e.endpoint();
        received_data = e.data();
    });
    
    dispatcher.dispatch(network::ConnectionEvent{
        network::ConnectionEvent::Type::DataReceived,
        "127.0.0.1:8080",
        "Hello from client"
    });
    
    EXPECT_EQ(received_type, network::ConnectionEvent::Type::DataReceived);
    EXPECT_EQ(received_endpoint, "127.0.0.1:8080");
    EXPECT_EQ(received_data, "Hello from client");
}

TEST(CommonEventsTest, EventInheritance) {
    // Verify that common events properly inherit from Event
    EXPECT_TRUE(is_event_v<MessageEvent>);
    EXPECT_TRUE(is_event_v<DataEvent>);
    EXPECT_TRUE(is_event_v<PropertyChangeEvent>);
    EXPECT_TRUE(is_event_v<SystemEvent>);
    EXPECT_TRUE(is_event_v<input::KeyEvent>);
    EXPECT_TRUE(is_event_v<input::MouseEvent>);
    EXPECT_TRUE(is_event_v<network::ConnectionEvent>);
}

TEST(CommonEventsTest, EventTimestamps) {
    auto start_time = std::chrono::steady_clock::now();
    
    MessageEvent event{"test"};
    
    auto end_time = std::chrono::steady_clock::now();
    
    EXPECT_GE(event.timestamp(), start_time);
    EXPECT_LE(event.timestamp(), end_time);
    EXPECT_FALSE(event.is_consumed());
}

TEST(CommonEventsTest, EventConsumption) {
    MessageEvent event{"test"};
    
    EXPECT_FALSE(event.is_consumed());
    
    event.consume();
    
    EXPECT_TRUE(event.is_consumed());
}