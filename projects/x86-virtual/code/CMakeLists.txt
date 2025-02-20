cmake_minimum_required(VERSION 3.22)
project(x86-virtual LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    add_subproject("shared")
    add_subproject("x86")
    add_subproject("abstraction/memory/virtual")
    add_subproject("abstraction/memory/physical")
    add_subproject("abstraction/memory/manipulation")
    add_subproject("abstraction/log")
    include("${REPO_PROJECTS}/print-configuration.cmake")
endif()

add_library(${PROJECT_NAME} OBJECT "src/x86-virtual.c")

add_includes_for_sublibrary()

add_subdirectory(status)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-physical-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
