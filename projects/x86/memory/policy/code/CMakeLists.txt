cmake_minimum_required(VERSION 3.22)
project(x86-memory-policy LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    add_project("shared")
    add_project("efi-to-kernel")
    add_project("x86")
    add_project("x86/memory/physical")
    add_project("x86/memory/virtual")
    add_project("abstraction/interrupts")
    add_project("abstraction/memory/management")
    add_project("abstraction/memory/manipulation")
    add_project("abstraction/memory/physical")
    add_project("abstraction/memory/virtual")
    add_project("abstraction/log")
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

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory-physical-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory-virtual-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
