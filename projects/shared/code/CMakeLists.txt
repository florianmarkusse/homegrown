cmake_minimum_required(VERSION 3.22)
project(shared LANGUAGES C ASM)
include("../../setup.cmake")
include("../../print-configuration.cmake")

add_library(
    ${PROJECT_NAME}
    STATIC
    "src/allocator/arena.c"
    "src/allocator/buddy.c"
    "src/allocator/pool.c"
    "src/manipulation/manipulation.c"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
