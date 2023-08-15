cmake_minimum_required(VERSION 3.17)
project(main)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

set(CMAKE_CXX_STANDARD 17)

add_executable(main main.cpp)

find_package(SDL2 REQUIRED)
target_link_libraries(main SDL2::Main SDL2)
