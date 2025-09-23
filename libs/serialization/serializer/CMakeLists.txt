# Create library
add_library(wip_serialization_serializer STATIC)
target_sources(wip_serialization_serializer PRIVATE src/serializer.cpp)
target_include_directories(wip_serialization_serializer PUBLIC include)
target_compile_features(wip_serialization_serializer PUBLIC cxx_std_17)

# Create alias for easier linking
add_library(wip::serialization::serializer ALIAS wip_serialization_serializer)

# Add tests if enabled
if(BUILD_TESTS)
    add_executable(test_wip_serialization_serializer test/test_serializer.cpp)
    target_link_libraries(test_wip_serialization_serializer PRIVATE 
        wip::serialization::serializer 
        GTest::gtest_main
    )
    
    # Add test to CTest
    add_test(NAME test_wip_serialization_serializer COMMAND test_wip_serialization_serializer)
endif()
