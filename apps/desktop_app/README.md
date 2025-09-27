# Desktop Application

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
- Example controls: sliders, checkboxes, text inputs
- **Layout**: 25% width, 100% height (right side)
- Perfect for application settings and properties
- Easily extensible for your application needs

#### Console Panel
- Log output and messages
- **Layout**: 75% width, 20% height (bottom area)
- Perfect for debugging and status information
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
- **Professional Docking**: Full ImGui docking branch with modern UI features
- **Multi-viewport Support**: Windows can be dragged outside the main application window
- **Event-driven**: Proper event handling for responsive applications
- **Extensible**: Easy to add new functionality without breaking existing code

## Next Steps

1. **Replace placeholder content** in `render_main_content()` with your application UI
2. **Add your application state** as member variables in `MainAppLayer`
3. **Implement file operations** using the File menu callbacks
4. **Add application-specific event handling** in `on_event()`
5. **Integrate with WIP utility libraries** as needed for your application

Your desktop application is ready for development! 🚀