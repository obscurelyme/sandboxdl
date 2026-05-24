cmake_minimum_required(VERSION 3.10)

set(SDL_MAIN_USE_CALLBACKS 0)

project(sandboxdl)

find_package(tinyxml2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

add_executable(sandboxdl 
  src/main.cpp
  src/logging/handler.cpp
  src/platform/spritesheet.cpp
  src/platform/input.cpp
  src/game/bricks.cpp
  src/game/bumper.cpp
)

target_include_directories(sandboxdl PRIVATE include)
target_link_libraries(sandboxdl PRIVATE tinyxml2::tinyxml2 fmt::fmt SDL3::SDL3)