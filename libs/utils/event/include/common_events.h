#pragma once

#include "event.h"
#include <string>
#include <any>

namespace wip::utils::event {

/**
 * @brief Common event types for typical applications.
 * 
 * These are concrete event implementations that can be used directly
 * or serve as examples for creating custom events.
 */

/**
 * @brief Generic message event for simple string-based communication.
 */
class MessageEvent : public Event {
public:
    explicit MessageEvent(std::string message) 
        : message_(std::move(message)) {}
    
    const std::string& message() const noexcept {
        return message_;
    }

private:
    std::string message_;
};

/**
 * @brief Generic data event that can carry arbitrary data.
 */
class DataEvent : public Event {
public:
    template<typename T>
    explicit DataEvent(std::string key, T&& data)
        : key_(std::move(key)), data_(std::forward<T>(data)) {}
    
    const std::string& key() const noexcept {
        return key_;
    }
    
    template<typename T>
    const T& data() const {
        return std::any_cast<const T&>(data_);
    }
    
    template<typename T>
    bool has_data() const noexcept {
        try {
            std::any_cast<const T&>(data_);
            return true;
        } catch (const std::bad_any_cast&) {
            return false;
        }
    }

private:
    std::string key_;
    std::any data_;
};

/**
 * @brief Property change event for observing object property modifications.
 */
class PropertyChangeEvent : public Event {
public:
    PropertyChangeEvent(std::string property_name, std::any old_value, std::any new_value)
        : property_name_(std::move(property_name))
        , old_value_(std::move(old_value))
        , new_value_(std::move(new_value)) {}
    
    const std::string& property_name() const noexcept {
        return property_name_;
    }
    
    template<typename T>
    const T& old_value() const {
        return std::any_cast<const T&>(old_value_);
    }
    
    template<typename T>
    const T& new_value() const {
        return std::any_cast<const T&>(new_value_);
    }

private:
    std::string property_name_;
    std::any old_value_;
    std::any new_value_;
};

/**
 * @brief System event for application lifecycle events.
 */
class SystemEvent : public Event {
public:
    enum class Type {
        Startup,
        Shutdown,
        Suspend,
        Resume,
        Error,
        Warning,
        Info
    };
    
    SystemEvent(Type type, std::string message = "")
        : type_(type), message_(std::move(message)) {}
    
    Type type() const noexcept {
        return type_;
    }
    
    const std::string& message() const noexcept {
        return message_;
    }

private:
    Type type_;
    std::string message_;
};

/**
 * @brief User input events for GUI applications.
 */
namespace input {

class KeyEvent : public Event {
public:
    enum class Type { Press, Release, Repeat };
    
    KeyEvent(Type type, int key_code, int modifiers = 0)
        : type_(type), key_code_(key_code), modifiers_(modifiers) {}
    
    Type type() const noexcept { return type_; }
    int key_code() const noexcept { return key_code_; }
    int modifiers() const noexcept { return modifiers_; }

private:
    Type type_;
    int key_code_;
    int modifiers_;
};

class MouseEvent : public Event {
public:
    enum class Type { Press, Release, Move, Scroll };
    
    MouseEvent(Type type, double x, double y, int button = -1)
        : type_(type), x_(x), y_(y), button_(button) {}
    
    Type type() const noexcept { return type_; }
    double x() const noexcept { return x_; }
    double y() const noexcept { return y_; }
    int button() const noexcept { return button_; }

private:
    Type type_;
    double x_, y_;
    int button_;
};

} // namespace input

/**
 * @brief Network-related events.
 */
namespace network {

class ConnectionEvent : public Event {
public:
    enum class Type { Connected, Disconnected, Error, DataReceived };
    
    ConnectionEvent(Type type, std::string endpoint, std::string data = "")
        : type_(type), endpoint_(std::move(endpoint)), data_(std::move(data)) {}
    
    Type type() const noexcept { return type_; }
    const std::string& endpoint() const noexcept { return endpoint_; }
    const std::string& data() const noexcept { return data_; }

private:
    Type type_;
    std::string endpoint_;
    std::string data_;
};

} // namespace network

} // namespace wip::utils::event