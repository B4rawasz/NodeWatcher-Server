add_subdirectory(server)
add_subdirectory(linux)
add_subdirectory(cli)

add_executable(NodeWatcher-Server main.cpp)

target_link_libraries(
    NodeWatcher-Server
    nodewatcher_server
    nodewatcher_linux
    nodewatcher_cli
    uWebSockets
)