# Create playground executable
add_executable(playground)
target_sources(playground PRIVATE src/main.cpp)
target_include_directories(playground PRIVATE include)
target_link_libraries(playground PRIVATE 
    wip_cli_args
    wip::utils::file 
    wip::utils::rng
    wip::utils::string
    wip::utils::uid
    wip::serialization::json_serializer
    wip::game::dice
    nlohmann_json::nlohmann_json
)

# Create RNG demo executable
add_executable(rng_demo)
target_sources(rng_demo PRIVATE src/rng_demo.cpp)
target_include_directories(rng_demo PRIVATE include)
target_link_libraries(rng_demo PRIVATE 
    wip::utils::rng
)

