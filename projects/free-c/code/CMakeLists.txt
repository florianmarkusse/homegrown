cmake_minimum_required(VERSION 3.22)
project(free-c LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(${REPO_PROJECTS}/setup.cmake)
    include(${REPO_PROJECTS}/print-configuration.cmake)
    add_subproject("shared")
    add_subproject("platform-abstraction")
endif()

add_subdirectory(memory)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
