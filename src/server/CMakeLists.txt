find_library(UUID_LIB uuid REQUIRED)

add_library(server STATIC
    core/server.cpp
    core/server_handlers.cpp
    auth/auth.cpp
)

target_include_directories(server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/auth
    ${CMAKE_CURRENT_SOURCE_DIR}/json
)

target_link_libraries(server PUBLIC
    uWebSockets
    nlohmann_json::nlohmann_json
    ${UUID_LIB}
)
