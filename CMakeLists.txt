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

project(breakout VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  include(FetchContent)

  FetchContent_Declare(
    tracy
    GIT_REPOSITORY  https://github.com/wolfpld/tracy.git
    GIT_TAG         master
    GIT_SHALLOW     TRUE
    GIT_PROGRESS    TRUE
  )

  FetchContent_MakeAvailable(tracy)

  target_compile_options(
    TracyClient 
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0 /WX->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w -Wno-error>
  )
endif()

find_package(box2d CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

add_executable(breakout 
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
  src/platform/physics.cpp
)

if(WIN32)
  set(APP_ICON_PATH "${CMAKE_SOURCE_DIR}/assets/app.ico" CACHE FILEPATH "Path to Windows .ico application icon")
  get_filename_component(APP_ICON_PATH_ABS "${APP_ICON_PATH}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
  if(EXISTS "${APP_ICON_PATH_ABS}")
    file(TO_CMAKE_PATH "${APP_ICON_PATH_ABS}" APP_ICON_PATH_CMAKE)
    set(WINDOWS_APP_ICON_RC "${CMAKE_BINARY_DIR}/windows-app-icon.rc")
    file(WRITE "${WINDOWS_APP_ICON_RC}" "IDI_APP_ICON ICON \"${APP_ICON_PATH_CMAKE}\"\n")
    target_sources(breakout PRIVATE "${WINDOWS_APP_ICON_RC}")
  else()
    message(STATUS "Windows app icon not found at '${APP_ICON_PATH_ABS}'. Set APP_ICON_PATH to a valid .ico file to embed an executable icon.")
  endif()
endif()

target_include_directories(breakout PRIVATE include)

target_link_libraries(
  breakout 
  PRIVATE 
  tinyxml2::tinyxml2 
  fmt::fmt 
  SDL3::SDL3 
  SDL3_ttf::SDL3_ttf
  box2d::box2d
)

if (BREAKOUT_ENABLE_PROFILING)
  message("-- BREAKOUT_ENABLE_PROFILING: ON")
  target_compile_definitions(breakout PRIVATE -DBREAKOUT_ENABLE_PROFILING)
else()
  message("-- BREAKOUT_ENABLE_PROFILING: OFF")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_link_libraries(
    breakout
    PRIVATE
    TracyClient
  )
endif()

if(WIN32)
  set_target_properties(breakout PROPERTIES
    WIN32_EXECUTABLE $<$<CONFIG:Release>:TRUE>
  )

  # Keep a windowed Release build (no console) while still using int main(...).
  target_link_options(breakout PRIVATE
    $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
  )
endif()

# Install the runtime executable and the full assets directory.
install(TARGETS breakout
  RUNTIME DESTINATION .
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
  DESTINATION .
)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)

include(CPack)