cmake_minimum_required(VERSION 3.22)
project(interoperation LANGUAGES C)
include("../../setup.cmake")

add_executable(${PROJECT_NAME} "src/main.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
