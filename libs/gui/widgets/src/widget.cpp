#include "widget.h"
#include <sstream>

namespace wip::gui {

// Static member initialization
int Widget::next_id_ = 1;

Widget::Widget(const std::string& id) {
    if (id.empty()) {
        // Generate unique ID if none provided
        id_ = "widget_" + std::to_string(next_id_++);
    } else {
        id_ = id;
    }
}

std::string Widget::make_imgui_id(const std::string& suffix) const {
    if (suffix.empty()) {
        return "##" + id_;
    }
    return suffix + "##" + id_;
}

} // namespace wip::gui