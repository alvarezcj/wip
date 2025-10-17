#pragma once

/**
 * @file widgets.h
 * @brief Main header for the WIP GUI Widgets library
 * 
 * This header provides convenient access to all widget classes in the
 * wip::gui namespace. Include this header to access Widget, Panel, and
 * other GUI abstractions.
 */

#include "widget.h"
#include "panel.h"
#include "selectable_text_widget.h"

namespace wip::gui {

// Type aliases for convenience
using WidgetPtr = std::unique_ptr<Widget>;
using PanelPtr = std::unique_ptr<Panel>;

/**
 * @brief Create a unique pointer to a widget
 * @tparam T Widget type (must derive from Widget)
 * @tparam Args Constructor argument types
 * @param args Constructor arguments
 * @return std::unique_ptr<T> Unique pointer to the created widget
 */
template<typename T, typename... Args>
std::unique_ptr<T> make_widget(Args&&... args) {
    static_assert(std::is_base_of_v<Widget, T>, "T must derive from Widget");
    return std::make_unique<T>(std::forward<Args>(args)...);
}

/**
 * @brief Create a unique pointer to a panel
 * @tparam T Panel type (must derive from Panel)
 * @tparam Args Constructor argument types
 * @param args Constructor arguments
 * @return std::unique_ptr<T> Unique pointer to the created panel
 */
template<typename T, typename... Args>
std::unique_ptr<T> make_panel(Args&&... args) {
    static_assert(std::is_base_of_v<Panel, T>, "T must derive from Panel");
    return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace wip::gui