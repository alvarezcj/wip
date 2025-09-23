# Create playground executable
add_executable(playground)
target_sources(playground PRIVATE src/main.cpp)
target_include_directories(playground PRIVATE include)
target_link_libraries(playground PRIVATE 
    wip::utils::file 
    wip::utils::string
    wip::serialization::json_serializer
    nlohmann_json::nlohmann_json
)

