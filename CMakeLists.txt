cmake_minimum_required(VERSION 3.23)
project(dsptrackmusplayer LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)

add_library(${PROJECT_NAME} SHARED "src/main.c")
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
pkg_check_modules(libopenmpt REQUIRED IMPORTED_TARGET libopenmpt)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::libopenmpt)
