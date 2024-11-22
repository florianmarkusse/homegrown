cmake_minimum_required(VERSION 3.22)
project(interoperation LANGUAGES C)
include("../../setup.cmake")
include("../../print-configuration.cmake")
add_link_options(--ld-path=${CMAKE_LINKER} -Wl,${COMMON_LINKER_FLAGS})

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
