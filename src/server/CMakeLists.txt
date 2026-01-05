find_library(UUID_LIB uuid REQUIRED)

add_library(nodewatcher_messages INTERFACE)

target_include_directories(nodewatcher_messages INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/json
)

target_link_libraries(nodewatcher_messages INTERFACE
    nlohmann_json::nlohmann_json
)

add_library(nodewatcher_server STATIC
    core/server.cpp
    core/server_handlers.cpp
    auth/auth.cpp
)

target_include_directories(nodewatcher_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/auth
)

target_link_libraries(nodewatcher_server PUBLIC
    uWebSockets
    nlohmann_json::nlohmann_json
    nodewatcher_messages
    nodewatcher_linux
    nodewatcher_events
    ${UUID_LIB}
)
