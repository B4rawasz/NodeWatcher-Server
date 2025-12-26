add_executable(NodeWatcher-Server main.cpp)

target_include_directories(NodeWatcher-Server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    NodeWatcher-Server
    uWebSockets
)