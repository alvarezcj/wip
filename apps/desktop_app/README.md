# De## Features

- **🔤 Custom Figtree font integration** - Beautiful modern typography with Regular, Medium, and Bold weights
- **🎨 Modern customizable theme system** with real-time color adjustments
- **🖼️ Professional docking interface** with drag-and-drop window management
- **📐 Consistent layout** - Windows always appear in the same position and proportions on startup
- **🌈 5 beautiful preset themes** - Dark Blue, Forest Green, Ember Orange, Rose Pink, Electric Purple
- **⚙️ Live theme editor** - Customize main, secondary, and accent colors in real-time
- **📋 Complete menu system** with File, View, and Help menus
- **🔧 Multiple dockable panels**: Main Content (75% width, 80% height), Properties (25% width, full height), Console (75% width, 20% height)
- **⚡ Event handling** for keyboard shortcuts and application events
- **🏗️ Layer-based architecture** for clean code organization
- **🎯 ImGui docking branch** with multi-viewport support
- **🖱️ Professional UI patterns** ready for extensionication

A professionally scaffolded desktop application built with the WIP C++ Framework, featuring complete docking support and a clean layer-based architecture.

## Features

- **🖼️ Professional docking interface** with drag-and-drop window management
- **� Consistent layout** - Windows always appear in the same position and proportions on startup
- **�📋 Complete menu system** with File, View, and Help menus
- **🔧 Multiple dockable panels**: Main Content (75% width, 80% height), Properties (25% width, full height), Console (75% width, 20% height)
- **⚡ Event handling** for keyboard shortcuts and application events
- **🏗️ Layer-based architecture** for clean code organization
- **🎯 ImGui docking branch** with multi-viewport support
- **🖱️ Professional UI patterns** ready for extension

## Structure

### MainAppLayer Class
The main application layer inherits from `Layer` and implements:

- **`on_attach()`** - Called when layer is added to the application
- **`on_detach()`** - Called when layer is removed from the application  
- **`on_update(Timestep)`** - Called every frame for logic updates
- **`on_render(Timestep)`** - Called every frame for UI rendering
- **`on_event(Event&)`** - Called for input and window events

### Application Panels

#### Main Content Panel
- Welcome information and framework overview
- **Layout**: 75% width, 80% height (top-left area)
- Interactive examples (counter, etc.)
- Framework integration documentation
- Ready for your main application content

#### Properties Panel
- **🔤 Font System Information**:
  - Real-time font loading status for Figtree family
  - Font samples showing Regular, Medium, and Bold weights
  - Visual feedback for successful font loading
- **🎨 Theme Customization Section**:
  - Real-time color pickers for main, secondary, and accent colors
  - 5 beautiful preset themes with one-click application
  - Live preview of all theme changes
- **⚙️ Example Controls Section**:
  - Modern styled sliders, checkboxes, text inputs
  - Demonstration of themed UI components
- **Layout**: 25% width, 100% height (right side)
- Perfect for application settings and theme customization
- Easily extensible for your application needs

#### Console Panel
- **📱 Modern styled log output** with color-coded message types
- **💬 Interactive command input** with Enter-to-execute functionality
- Real-time theme debugging information display
- **Layout**: 75% width, 20% height (bottom area)
- Perfect for debugging, logging, and user commands
- Ready for integration with your logging system

## Consistent Docking Layout

The application automatically sets up a consistent window layout on every startup:

```
┌─────────────────────────┬─────────────┐
│                         │             │
│     Main Content        │ Properties  │
│     (75% × 80%)         │ (25% × 100%)│
│                         │             │
├─────────────────────────┤             │
│     Console             │             │
│     (75% × 20%)         │             │
└─────────────────────────┴─────────────┘
```

### Layout Details:
- **Main Content**: Top-left, takes 75% width and 80% height
- **Properties**: Right side, takes 25% width and 100% height  
- **Console**: Bottom-left, takes 75% width and 20% height
- All windows start docked and positioned consistently every time

### Implementation:
The layout is enforced using `ImGui::SetNextWindowPos()` and `ImGui::SetNextWindowSize()` on the first frame only. After that, users can drag and rearrange windows as needed, but they'll return to the default layout on next startup.

## 🎨 Modern Theme System

The application features a comprehensive theming system with real-time customization capabilities:

### Theme Structure
```cpp
struct ModernTheme {
    ImVec4 main_color;      // Window backgrounds, primary surfaces
    ImVec4 secondary_color; // Controls, buttons, input fields  
    ImVec4 accent_color;    // Highlights, active elements, focus indicators
    ImVec4 text_color;      // Primary text color
    ImVec4 text_disabled;   // Disabled text color
    ImVec4 background;      // Deep background color
};
```

### Built-in Preset Themes

| Theme | Description | Colors |
|-------|-------------|---------|
| 🌙 **Dark Blue** | Professional dark theme with blue accents | Deep blue-gray + bright blue highlights |
| 🌿 **Forest Green** | Nature-inspired dark green theme | Dark forest + bright green accents |
| 🔥 **Ember Orange** | Warm dark theme with orange highlights | Dark red-brown + vibrant orange |
| 🌸 **Rose Pink** | Elegant dark theme with pink accents | Dark rose + soft pink highlights |
| ⚡ **Electric Purple** | Modern dark theme with purple energy | Dark purple + electric purple accents |

### Real-time Customization
- **Live color pickers** for main, secondary, and accent colors
- **Instant preview** - see changes applied immediately
- **One-click presets** - switch between themes instantly
- **Consistent styling** - all UI elements automatically adapt

### Modern UI Features
- **Rounded corners** on all elements (6px windows, 4px controls)
- **Smooth hover effects** with color transitions
- **Proper spacing** and padding for professional look
- **Color-coded console output** (Info: Green, Warning: Yellow, Debug: Blue, System: Gray)
- **Styled components** - buttons, sliders, inputs all themed consistently

### Custom Theme Implementation
```cpp
// Apply a custom theme
apply_modern_theme(
    ImVec4(0.15f, 0.16f, 0.21f, 1.00f), // Main color (dark blue-gray)
    ImVec4(0.20f, 0.22f, 0.27f, 1.00f), // Secondary color (lighter blue-gray)  
    ImVec4(0.26f, 0.59f, 0.98f, 1.00f)  // Accent color (bright blue)
);
```

The theme system automatically handles:
- Window backgrounds and borders
- Button states (normal, hovered, active)
- Input field styling
- Scrollbar appearance
- Tab and header styling
- Docking preview colors
- Text selection highlights

## 🔤 Figtree Font System

The application uses the modern **Figtree font family** for beautiful, professional typography:

### Font Weights Available
- **Figtree Regular** (16px) - Default font for all UI text
- **Figtree Medium** (18px) - Used for headers and section titles
- **Figtree Bold** (16px) - Used for emphasis and important elements

### Font Loading Features
- **Automatic fallback** - Uses system default fonts if Figtree files aren't found
- **Smart loading** - Loads fonts from `Figtree/static/` directory
- **Visual feedback** - Shows loading status in Properties panel
- **Font samples** - Live preview of all loaded font weights

### Font File Structure Expected
```
Figtree/
├── static/
│   ├── Figtree-Regular.ttf    (Default UI font)
│   ├── Figtree-Medium.ttf     (Headers)
│   └── Figtree-Bold.ttf       (Emphasis)
└── ...
```

### Font System Implementation
```cpp
struct FontSystem {
    ImFont* regular_font;  // Default text
    ImFont* medium_font;   // Headers  
    ImFont* bold_font;     // Emphasis
    
    void load_fonts(); // Automatic font loading with fallbacks
};

// Usage in render code
ImGui::PushFont(font_system.medium_font, 0.0f);
ImGui::Text("Header Text");
ImGui::PopFont();
```

### Typography Hierarchy
- **Welcome titles**: Medium weight, larger size
- **Section headers**: Bold weight for emphasis  
- **Body text**: Regular weight (default)
- **Console headers**: Bold weight for visibility
- **UI controls**: Regular weight for consistency

## Controls & Shortcuts

- **ESC** - Quit application
- **F1** - Help message in console
- **F11** - Toggle fullscreen (placeholder for implementation)
- **Mouse** - Drag window tabs to dock/undock panels
- **Menu shortcuts**: Ctrl+N (New), Ctrl+O (Open), Ctrl+S (Save), Alt+F4 (Exit)

## Building & Running

```bash
# Build the desktop application
cd build
cmake ..
make desktop_app

# Run the application
./bin/desktop_app
```

## Extending the Application

### Adding Your Logic

Add your application logic in the `MainAppLayer` class:

1. **Update Logic**: Add to `on_update(Timestep timestep)` method
   ```cpp
   void on_update(Timestep timestep) override {
       // Your game/app logic here
       // timestep.delta_time() gives frame delta time
       // timestep.total_time() gives total elapsed time
   }
   ```

2. **UI Rendering**: Modify render methods or create new ones
   ```cpp
   void render_main_content() {
       ImGui::Begin("Your Panel");
       // Your UI code here
       ImGui::End();
   }
   ```

3. **Event Handling**: Add cases in `on_event()` method
   ```cpp
   bool on_event(const wip::utils::event::Event& event) override {
       if (auto* key_event = dynamic_cast<const KeyboardEvent*>(&event)) {
           // Handle keyboard events
       }
       return false; // Return true to consume event
   }
   ```

4. **Custom Themes**: Create your own themes
   ```cpp
   // Apply custom theme colors
   apply_modern_theme(
       ImVec4(0.12f, 0.15f, 0.18f, 1.00f), // Your main color
       ImVec4(0.18f, 0.22f, 0.25f, 1.00f), // Your secondary color  
       ImVec4(0.85f, 0.35f, 0.25f, 1.00f)  // Your accent color
   );
   ```

### Adding New Panels

1. Create a new render method:
   ```cpp
   void render_your_panel() {
       ImGui::Begin("Your Panel Name");
       // Your panel content
       ImGui::End();
   }
   ```

2. Call it from `on_render()`:
   ```cpp
   void on_render(Timestep timestep) override {
       // ... existing code ...
       render_your_panel(); // Add this line
   }
   ```

## Framework Integration

This application demonstrates full integration with:

- **WIP GUI Application framework** - Complete application lifecycle management
- **WIP Window management** - Cross-platform window creation and management
- **WIP Event system** - Type-safe event handling and dispatching
- **ImGui with docking support** - Professional UI with drag-and-drop docking
- **OpenGL rendering** - Hardware-accelerated graphics
- **All utility libraries** available:
  - `wip::utils::rng` - Random number generation
  - `wip::utils::string` - String utilities
  - `wip::utils::file` - File operations
  - `wip::game::dice` - Dice rolling utilities
  - `wip::serialization` - JSON serialization support

## Architecture Benefits

- **Clean Separation**: ImGui lifecycle managed by Application, not layers
- **Simplified Layer System**: No special ImGui layer needed - just inherit from `Layer`
- **Modern Theme System**: Comprehensive theming with real-time customization
- **Professional Docking**: Full ImGui docking branch with modern UI features
- **Multi-viewport Support**: Windows can be dragged outside the main application window
- **Event-driven**: Proper event handling for responsive applications
- **Extensible**: Easy to add new functionality without breaking existing code
- **Consistent Styling**: All UI elements automatically adapt to theme changes

## Next Steps

1. **Replace placeholder content** in `render_main_content()` with your application UI
2. **Add your application state** as member variables in `MainAppLayer`
3. **Implement file operations** using the File menu callbacks
4. **Add application-specific event handling** in `on_event()`
5. **Integrate with WIP utility libraries** as needed for your application

Your desktop application is ready for development! 🚀