cmake_minimum_required(VERSION 3.17)
project(viewer)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

set(CMAKE_CXX_STANDARD 17)

add_executable(viewer main.cpp Graphics.cpp Graphics.h Renderer.cpp Renderer.h CgfCreator.cpp CgfCreator.h)

find_package(SDL2 REQUIRED)
target_link_libraries(viewer SDL2::Main SDL2)
