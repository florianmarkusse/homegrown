cmake_minimum_required(VERSION 3.22)
project(freestanding LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(${REPO_PROJECTS}/setup.cmake)

    add_project("shared")
    add_project("efi-to-kernel")
    add_project("abstraction/log")
    add_project("abstraction/memory/virtual")
    add_project("abstraction/memory/manipulation")
    add_project("abstraction/memory/management")

    include(${REPO_PROJECTS}/print-configuration.cmake)
endif()

add_subdirectory(memory)
add_subdirectory(peripheral)
add_subdirectory(log)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
