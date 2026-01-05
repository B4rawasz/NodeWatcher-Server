add_library(nodewatcher_events STATIC
    event_bus.cpp
)

target_include_directories(nodewatcher_events PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nodewatcher_events PUBLIC
    nodewatcher_messages
)