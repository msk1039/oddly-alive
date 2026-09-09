cmake_minimum_required(VERSION 3.20)
project(procedural_koi VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib)

add_executable(
  procedural-koi
  src/main.cpp
  src/koi.cpp
  src/renderer.cpp
  src/school.cpp
)
target_link_libraries(procedural-koi PRIVATE PkgConfig::RAYLIB)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(procedural-koi PRIVATE -Wall -Wextra -Wpedantic)
endif()
