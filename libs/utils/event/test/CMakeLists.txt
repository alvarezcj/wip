# Tests for the event system library

# Create the test executable
add_executable(test_wip_utils_event
    test_event_dispatcher.cpp
    test_common_events.cpp
    test_thread_safety.cpp
)

# Link against the event library and gtest
target_link_libraries(test_wip_utils_event
    PRIVATE
        wip_utils_event
        gtest_main
        gtest
)

# Set C++17 standard
target_compile_features(test_wip_utils_event PRIVATE cxx_std_17)

# Add compiler warnings
if(MSVC)
    target_compile_options(test_wip_utils_event PRIVATE /W4)
else()
    target_compile_options(test_wip_utils_event PRIVATE -Wall -Wextra -pedantic)
endif()

# Register the test
add_test(NAME test_wip_utils_event COMMAND test_wip_utils_event)

# Add a custom target to run only event system tests
add_custom_target(test_event_system
    COMMAND test_wip_utils_event
    DEPENDS test_wip_utils_event
    COMMENT "Running event system tests..."
)