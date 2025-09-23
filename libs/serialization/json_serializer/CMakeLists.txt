# Create library
add_library(wip_serialization_json_serializer STATIC)
target_sources(wip_serialization_json_serializer PRIVATE src/json_serializer.cpp)
target_include_directories(wip_serialization_json_serializer PUBLIC include)
target_compile_features(wip_serialization_json_serializer PUBLIC cxx_std_17)

# Link dependencies
target_link_libraries(wip_serialization_json_serializer PUBLIC 
    wip::serialization::serializer
    nlohmann_json::nlohmann_json
)

# Create alias for easier linking
add_library(wip::serialization::json_serializer ALIAS wip_serialization_json_serializer)

# Add tests if enabled
if(BUILD_TESTS)
    add_executable(test_wip_serialization_json_serializer test/test_json_serializer.cpp)
    target_link_libraries(test_wip_serialization_json_serializer PRIVATE 
        wip::serialization::json_serializer 
        GTest::gtest_main
    )
    
    # Add test to CTest
    add_test(NAME test_wip_serialization_json_serializer COMMAND test_wip_serialization_json_serializer)
endif()
