cmake_minimum_required(VERSION 3.22)
project(image-builder LANGUAGES C ASM)
include("${REPO_PROJECTS}/setup.cmake")
include("${REPO_PROJECTS}/print-configuration.cmake")
add_link_options("--ld-path=${CMAKE_LINKER}")
add_subproject("shared")

add_executable(${PROJECT_NAME} "src/main.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-sizes)
