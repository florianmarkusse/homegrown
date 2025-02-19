cmake_minimum_required(VERSION 3.22)
project(freestanding LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(${REPO_PROJECTS}/setup.cmake)

    add_subproject("shared")
    add_subproject("efi-to-kernel")
    add_subproject("abstraction/log")
    add_subproject("abstraction/memory/virtual")
    add_subproject("abstraction/memory/manipulation")
    add_subproject("abstraction/memory/management")

    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} --embed-dir=${REPO_PROJECTS}/${PROJECT_NAME}/resources"
    )

    include(${REPO_PROJECTS}/print-configuration.cmake)
endif()

add_subdirectory(memory)
add_subdirectory(peripheral)
add_subdirectory(log)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
