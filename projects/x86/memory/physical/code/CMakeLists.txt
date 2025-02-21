cmake_minimum_required(VERSION 3.22)
project(x86-memory-physical LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    add_project("shared")
    add_project("x86")
    add_project("efi-to-kernel")
    add_project("abstraction/memory/physical")
    add_project("abstraction/memory/manipulation")
    add_project("abstraction/log")
    add_project("abstraction/interrupts")
    include("${REPO_PROJECTS}/print-configuration.cmake")
endif()

add_library(${PROJECT_NAME} OBJECT "src/physical.c")

add_includes_for_sublibrary()

if("${UNIT_TEST_BUILD}")
    add_subdirectory(tests)
endif()

add_subdirectory(status)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-physical-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
