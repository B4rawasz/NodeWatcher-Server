cmake_minimum_required(VERSION 3.16)

project(NodeWatcher-Server VERSION 0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(NodeWatcher-Server)

target_sources(NodeWatcher-Server PRIVATE
    src/main.cpp
)