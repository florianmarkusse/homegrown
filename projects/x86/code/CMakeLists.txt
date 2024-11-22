cmake_minimum_required(VERSION 3.22)
project(x86 LANGUAGES C ASM)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("../../setup.cmake")
    include("../../print-configuration.cmake")
    add_link_options(--ld-path=${CMAKE_LINKER} -Wl,${COMMON_LINKER_FLAGS})
endif()

add_subdirectory(memory)
