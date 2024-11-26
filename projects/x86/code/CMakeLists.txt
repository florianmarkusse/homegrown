cmake_minimum_required(VERSION 3.22)
project(x86 LANGUAGES C ASM)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    add_link_options("--ld-path=${CMAKE_LINKER}")
    add_subproject("shared")
endif()

add_subdirectory(memory)
add_subdirectory(cpu)
