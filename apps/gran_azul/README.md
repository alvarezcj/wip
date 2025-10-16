# Gran Azul - C/C++ Code Quality Analysis Platform

Gran Azul is a desktop application for c### Dependencies

- **OpenGL**: Graphics rendering
- **X11**: Linux window system
- **ImGui**: GUI framework
- **GLFW**: Window and input management
- **Figtree Fonts**: Typography (optional, fallback to default)
- **wip::utils::process**: System command execution utility
- **pthread**: Threading support for process executionnsive C/C++ code quality analysis. It serves as an aggregator and facilitator for established static analysis tools, providing unified reporting, quality gates, and actionable insights.

## Overview

This is the desktop GUI component of the Gran Azul code quality platform. It provides a modern, user-friendly interface for:

- Opening and analyzing C/C++ projects
- Running comprehensive code quality checks
- Viewing detailed analysis results
- Configuring quality gates and rules
- Exporting reports and metrics

## Features

### Current Implementation (v1.0.1)
- ✅ **Modern GUI Framework**: Built with ImGui and the WIP GUI framework
- ✅ **Gran Azul Theme**: Custom blue color scheme matching the brand (no emojis)
- ✅ **Font System**: Integrated Figtree font family for professional appearance
- ✅ **Dockable Interface**: Flexible window layout with ImGui docking
- ✅ **Menu System**: File, Analysis, View, and Help menus with functionality
- ✅ **Application Framework**: Proper layer-based architecture
- ✅ **Process Execution**: Integrated system command execution utility
- ✅ **Log Window**: Real-time command output logging with timestamps
- ✅ **cppcheck Integration**: Test cppcheck availability and version

### Planned Features
- 🔄 **cppcheck Integration**: Static analysis and bug detection
- 🔄 **clang-tidy Integration**: Clang-based linting and modernization
- 🔄 **Project Management**: Open, scan, and manage C/C++ projects
- 🔄 **Quality Gates**: Configurable pass/fail criteria
- 🔄 **Report Generation**: Export analysis results in multiple formats
- 🔄 **MISRA Compliance**: Support for MISRA C/C++ standards
- 🔄 **Security Analysis**: Vulnerability detection and reporting

## Building

The application is built as part of the main WIP project:

```bash
cd build
cmake ..
make gran_azul
```

## Running

Execute the application from the project root:

```bash
./build/bin/gran_azul
```

## Architecture

### Application Structure
- **GranAzulApp**: Main application class managing windows and layers
- **GranAzulMainLayer**: Primary UI layer handling rendering and events
- **GranAzulTheme**: Custom color scheme and styling
- **FontSystem**: Figtree font family integration

### Framework Integration
- **WIP GUI Framework**: Uses `wip::gui::application` and `wip::gui::window`
- **ImGui**: Modern immediate-mode GUI with docking support
- **Event System**: Keyboard and mouse event handling via `wip::utils::event`
- **Layer Architecture**: Modular design for adding analysis components

### Controls
- **ESC**: Quit application
- **F1**: Show help
- **F11**: Toggle fullscreen
- **Ctrl+O**: Open project (planned)
- **F5**: Run analysis (planned)

### New Features
- **Log Window**: View -> Log Window to show/hide command execution log
- **cppcheck Test**: Analysis -> Test cppcheck --version to verify tool availability
- **Clear Log**: Button in log window to clear all command history
- **Command History**: Timestamped log entries with exit codes and duration

## Code Standards

This application follows the established WIP project conventions:

- **C++17**: Modern C++ standards
- **CMake**: Build system integration
- **Namespace Usage**: Proper `using` declarations
- **Resource Management**: RAII and smart pointers
- **Error Handling**: Exception-safe code with proper logging
- **Consistent Styling**: Matches existing desktop_app patterns

## Dependencies

- **OpenGL**: Graphics rendering
- **X11**: Linux window system
- **ImGui**: GUI framework
- **GLFW**: Window and input management
- **Figtree Fonts**: Typography (optional, fallback to default)

## Future Development

The next development phase will focus on integrating actual code analysis tools:

1. **cppcheck Integration** - Static analysis engine
2. **Project File Handling** - Open and parse C/C++ projects
3. **Analysis Pipeline** - Run tools and aggregate results
4. **Results Visualization** - Display findings with severity levels
5. **Report Export** - Generate professional reports

## Contributing

This application is part of the Gran Azul code quality business initiative. Development follows the patterns established in the WIP framework for consistency and maintainability.

---

**Version**: 1.0.1 (Process Integration)  
**Status**: Basic window application with command execution and logging  
**Next Phase**: Full project analysis pipeline