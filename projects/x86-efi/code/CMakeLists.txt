cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(x86-efi LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(${REPO_PROJECTS}/setup.cmake)
    add_subproject("shared")
    add_subproject("x86")
    add_subproject("x86-virtual")
    add_subproject("efi")
    add_subproject("efi-to-kernel")
    add_subproject("abstraction/log")
    add_subproject("abstraction/memory/manipulation")
    add_subproject("abstraction/memory/physical")
    include(${REPO_PROJECTS}/print-configuration.cmake)
endif()

add_library(${PROJECT_NAME} OBJECT "src/init.c" "src/jump.c" "src/gdt.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-physical-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-virtual-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
