add_subdirectory(server)

add_executable(NodeWatcher-Server main.cpp)

target_link_libraries(
    NodeWatcher-Server
    server
    uWebSockets
)