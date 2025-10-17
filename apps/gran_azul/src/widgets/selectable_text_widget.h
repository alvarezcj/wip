#pragma once

#include <widgets.h>
#include <string>
#include <functional>

namespace gran_azul::widgets {

using TextCopyCallback = std::function<void(const std::string&)>;

class SelectableTextWidget : public wip::gui::Widget {
private:
    std::string text_;
    std::string unique_id_;
    TextCopyCallback on_text_copied_;
    
public:
    SelectableTextWidget(const std::string& text = "", const std::string& unique_id = "");
    
    // Widget interface
    void update(float delta_time) override;
    void draw() override;
    
    // Text management
    void set_text(const std::string& text) { text_ = text; }
    const std::string& get_text() const { return text_; }
    
    void set_unique_id(const std::string& id) { unique_id_ = id; }
    const std::string& get_unique_id() const { return unique_id_; }
    
    // Callback
    void set_copy_callback(TextCopyCallback callback) { on_text_copied_ = callback; }
    
    // Static utility methods
    static void render_selectable_text_lines(const std::string& text, const std::string& base_id, TextCopyCallback copy_callback = nullptr);
    static void copy_text_to_clipboard(const std::string& text);
    
private:
    void render_text_lines();
};

} // namespace gran_azul::widgets