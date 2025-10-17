include(FetchContent)

# Download nativefiledialog-extended
FetchContent_Declare(
    nativefiledialog-extended
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG v1.2.1  # Latest stable version
    GIT_SHALLOW TRUE
)

# Configure nativefiledialog-extended options
set(NFD_BUILD_TESTS OFF CACHE BOOL "Build NFD tests" FORCE)
set(NFD_INSTALL OFF CACHE BOOL "Install NFD" FORCE)

# Make nativefiledialog-extended available
FetchContent_MakeAvailable(nativefiledialog-extended)

message(STATUS "nativefiledialog-extended configured successfully")