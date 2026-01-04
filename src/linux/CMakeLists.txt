add_library(nodewatcher_linux STATIC
    files/api_keys.cpp
)

target_include_directories(nodewatcher_linux PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/files
)

target_link_libraries(nodewatcher_linux PUBLIC
    nlohmann_json::nlohmann_json
)