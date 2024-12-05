cmake_minimum_required(VERSION 3.22)
project(x86 LANGUAGES C ASM)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    add_link_options("--ld-path=${CMAKE_LINKER}")
    add_subproject("shared")
    add_platform_abstraction_and_correct_implementations()
endif()

add_subdirectory(memory)
add_subdirectory(cpu)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
