cmake_minimum_required(VERSION 3.23)
project(dsptrackmusplayer LANGUAGES C)

find_package(PkgConfig REQUIRED)

add_library(${PROJECT_NAME} SHARED "src/main.c")
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
pkg_check_modules(libopenmpt REQUIRED IMPORTED_TARGET libopenmpt)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::libopenmpt)
