# Library target
add_library(wip_utils_event STATIC)
target_sources(wip_utils_event PRIVATE src/event_dispatcher.cpp)
target_include_directories(wip_utils_event PUBLIC include)
target_compile_features(wip_utils_event PUBLIC cxx_std_17)

# Find required packages
find_package(Threads REQUIRED)

# Link required libraries
target_link_libraries(wip_utils_event PUBLIC 
    Threads::Threads
)

# Alias for easier linking
add_library(wip::utils::event ALIAS wip_utils_event)

# Tests
if(BUILD_TESTS)
    add_executable(test_wip_utils_event 
        test/test_event_dispatcher.cpp
        test/test_common_events.cpp
        test/test_thread_safety.cpp
    )
    target_link_libraries(test_wip_utils_event PRIVATE 
        wip::utils::event
        GTest::gtest_main
    )
    add_test(NAME test_wip_utils_event COMMAND test_wip_utils_event)
endif()