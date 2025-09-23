include(FetchContent)

# Download nlohmann/json
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Configure nlohmann/json build options
set(JSON_BuildTests OFF CACHE BOOL "Build the unit tests when BUILD_TESTING is enabled" FORCE)
set(JSON_Install OFF CACHE BOOL "Install CMake targets during install step" FORCE)
set(JSON_MultipleHeaders OFF CACHE BOOL "Use non-amalgamated version of the library" FORCE)

# Make nlohmann/json available
FetchContent_MakeAvailable(nlohmann_json)

# Create convenience alias if it doesn't exist
if(NOT TARGET nlohmann_json::nlohmann_json)
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
endif()

message(STATUS "nlohmann/json configured successfully")