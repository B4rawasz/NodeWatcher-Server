include(FetchContent)

# =====================
# OpenSSL / ZLIB
# =====================
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# =====================
# uSockets
# =====================
FetchContent_Declare(
  uSockets_content
  GIT_REPOSITORY https://github.com/uNetworking/uSockets
  GIT_TAG v0.8.8
  GIT_SHALLOW ON
  GIT_SUBMODULES ""
)

FetchContent_MakeAvailable(uSockets_content)

file(GLOB_RECURSE SOURCES 
  ${usockets_content_SOURCE_DIR}/src/*.c
  ${usockets_content_SOURCE_DIR}/src/crypto/*.c*
  ${usockets_content_SOURCE_DIR}/src/eventing/*.c*
  ${usockets_content_SOURCE_DIR}/src/io_uring/*.c*
)

add_library(uSockets STATIC
    ${SOURCES}
)

target_compile_definitions(uSockets PRIVATE
    LIBUS_USE_OPENSSL
)

target_include_directories(uSockets PUBLIC
    ${usockets_content_SOURCE_DIR}/src
)

target_link_libraries(uSockets PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
)

# =====================
# uWebSockets
# =====================
FetchContent_Declare(
  uWebSockets_content
  GIT_REPOSITORY https://github.com/uNetworking/uWebSockets
  GIT_TAG v20.74.0
  GIT_SHALLOW ON
  GIT_SUBMODULES ""
)

FetchContent_MakeAvailable(uWebSockets_content)

add_library(uWebSockets INTERFACE)

target_include_directories(uWebSockets INTERFACE
    ${uwebsockets_content_SOURCE_DIR}/src/
)

target_link_libraries(uWebSockets INTERFACE
    uSockets
    ${ZLIB_LIBRARIES}
)

target_compile_options(uWebSockets INTERFACE
    -Wno-deprecated-declarations
)

# =====================
# nlohmann_json
# =====================
FetchContent_Declare(
    nlohmann_json_content
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json_content)