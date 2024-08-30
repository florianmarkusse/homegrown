cmake_minimum_required(VERSION 3.22)
project(interoperation LANGUAGES C)
include("../../setup.cmake")

add_executable(${MAIN_RESULT} "src/main.c")

target_include_directories(
    ${MAIN_RESULT}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
