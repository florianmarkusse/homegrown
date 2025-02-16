cmake_minimum_required(VERSION 3.22)
project(platform-abstraction LANGUAGES C)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
endif()

include("${REPO_PROJECTS}/abstractions.cmake")

add_subdirectory(log)
add_subdirectory(memory)

## NOTE: temporary until we improve the abstraction structure
add_subdirectory(physical)
add_subdirectory(virtual)
add_subdirectory(interrupts)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
