cmake_minimum_required(VERSION 3.22)
project(image-builder LANGUAGES C ASM)
include("../../setup.cmake")
include("../../print-configuration.cmake")

add_executable(${PROJECT_NAME} "src/main.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
