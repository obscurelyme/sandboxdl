cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

set(SDL_MAIN_USE_CALLBACKS 0)

if(WIN32)
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet" FORCE)
    set(VCPKG_HOST_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg host triplet" FORCE)
  elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg target triplet" FORCE)
    set(VCPKG_HOST_TRIPLET "x64-windows" CACHE STRING "Vcpkg host triplet" FORCE)
  endif()
endif()

project(sandboxdl)

find_package(tinyxml2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

add_executable(sandboxdl 
  src/main.cpp
  src/logging/handler.cpp
  src/platform/spritesheet.cpp
  src/platform/input.cpp
  src/game/bricks.cpp
  src/game/bumper.cpp
  src/platform/debug-gui.cpp
  src/game/ball.cpp
  src/platform/ui.cpp
  src/platform/text.cpp
  src/platform/audio.cpp
  src/platform/scene.cpp
  src/game/main-menu-scene.cpp
  src/game/credits-scene.cpp
  src/game/game-scene.cpp
  src/game/background.cpp
  src/game/player-lives.cpp
)

target_include_directories(sandboxdl PRIVATE include)

target_link_libraries(
  sandboxdl 
  PRIVATE 
  tinyxml2::tinyxml2 
  fmt::fmt 
  SDL3::SDL3 
  SDL3_ttf::SDL3_ttf
)