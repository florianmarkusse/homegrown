cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(efi LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")

    add_project("shared")
    add_project("efi-to-kernel")
    add_project("abstraction/memory/manipulation")
    add_project("abstraction/memory/physical")
    add_project("abstraction/log")

    include("${REPO_PROJECTS}/print-configuration.cmake")
endif()

add_subdirectory(acpi)
add_subdirectory(firmware)
add_subdirectory(globals)
add_subdirectory(log)
add_subdirectory(error)
add_subdirectory(memory)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
