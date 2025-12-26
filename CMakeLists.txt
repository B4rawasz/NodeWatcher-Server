cmake_minimum_required(VERSION 3.20)

project(NodeWatcher-Server VERSION 0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/deps.cmake)

add_subdirectory(src)