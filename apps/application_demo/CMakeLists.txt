find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

add_executable(application_demo src/main.cpp)
target_link_libraries(application_demo PRIVATE 
    wip::gui::application
    wip::gui::window
    wip::utils::event
    OpenGL::GL
)

add_executable(layer_demo src/layer_demo.cpp)
target_link_libraries(layer_demo PRIVATE 
    wip::gui::application
    wip::gui::window
    wip::utils::event
    OpenGL::GL
)

add_executable(imgui_layer_demo src/imgui_layer_demo.cpp)
target_link_libraries(imgui_layer_demo PRIVATE 
    wip::gui::application
    wip::gui::window
    wip::utils::event
    OpenGL::GL
    imgui::imgui
    ${X11_LIBRARIES}
)

add_executable(simple_imgui_demo src/simple_imgui_demo.cpp)
target_link_libraries(simple_imgui_demo PRIVATE 
    wip::gui::application
    wip::gui::window
    wip::utils::event
    OpenGL::GL
    imgui::imgui
    ${X11_LIBRARIES}
)

add_executable(docking_demo src/docking_demo.cpp)
target_link_libraries(docking_demo PRIVATE 
    wip::gui::application
    wip::gui::window
    wip::utils::event
    OpenGL::GL
    imgui::imgui
    ${X11_LIBRARIES}
)