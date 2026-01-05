add_library(nodewatcher_cli STATIC
    daemon.cpp
    keygen.cpp
    help.cpp
)

target_include_directories(nodewatcher_cli PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nodewatcher_cli PUBLIC
    nodewatcher_linux
    nodewatcher_server
    nodewatcher_events
)


