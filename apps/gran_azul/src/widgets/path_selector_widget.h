#pragma once

#include <widgets.h>
#include <string>
#include <functional>

namespace gran_azul::widgets {

enum class PathType {
    File,
    Folder
};

struct PathFilter {
    std::string name;     // Filter name (e.g., "C++ Files")
    std::string extension; // Filter extension (e.g., "cpp,h,hpp")
    
    PathFilter(const std::string& filter_name, const std::string& filter_ext)
        : name(filter_name), extension(filter_ext) {}
};

// Callback types
using PathSelectedCallback = std::function<void(const std::string&)>;

class PathSelectorWidget {
private:
    std::string label_;
    std::string current_path_;
    PathType path_type_;
    std::vector<PathFilter> filters_;
    PathSelectedCallback on_path_selected_;
    float label_width_;
    float button_width_;
    
public:
    PathSelectorWidget(const std::string& label, PathType type = PathType::Folder, float label_width = 120.0f);
    
    // Configuration
    void set_path(const std::string& path) { current_path_ = path; }
    const std::string& get_path() const { return current_path_; }
    
    void set_path_type(PathType type) { path_type_ = type; }
    void add_filter(const PathFilter& filter) { filters_.push_back(filter); }
    void clear_filters() { filters_.clear(); }
    
    void set_callback(PathSelectedCallback callback) { on_path_selected_ = callback; }
    
    // Rendering
    void draw();
    bool draw_inline(); // Returns true if path was changed
    
private:
    void open_dialog();
    std::string get_relative_path(const std::string& absolute_path, const std::string& base_path);
};

} // namespace gran_azul::widgets