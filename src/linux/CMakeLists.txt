add_library(nodewatcher_linux STATIC
    files/api_keys.cpp
    modules/static_resource.cpp
    modules/light_module.cpp
    modules/scheduler/scheduler.cpp
    modules/system/system.cpp
    modules/cpu/cpu.cpp
)

target_include_directories(nodewatcher_linux PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/files
    ${CMAKE_CURRENT_SOURCE_DIR}/modules
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/scheduler
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/system
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/cpu
)

target_link_libraries(nodewatcher_linux PUBLIC
    nodewatcher_server
    nlohmann_json::nlohmann_json
)