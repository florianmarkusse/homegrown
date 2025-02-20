cmake_minimum_required(VERSION 3.22)
project(x86-policy LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    add_subproject("shared")
    add_subproject("efi-to-kernel")
    add_subproject("x86")
    add_subproject("x86-physical")
    add_subproject("x86-virtual")
    add_subproject("abstraction/interrupts")
    add_subproject("abstraction/memory/management")
    add_subproject("abstraction/memory/manipulation")
    add_subproject("abstraction/memory/physical")
    add_subproject("abstraction/memory/virtual")
    add_subproject("abstraction/log")
    include("${REPO_PROJECTS}/print-configuration.cmake")
endif()

add_library(${PROJECT_NAME} OBJECT "src/init.c" "src/policy.c" "src/virtual.c")

add_includes_for_sublibrary()

add_subdirectory(status)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-management-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-physical-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-virtual-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
