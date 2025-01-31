cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(os-loader LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)
include("${REPO_PROJECTS}/setup.cmake")
include("${REPO_PROJECTS}/efi-setup.cmake")
include("${REPO_PROJECTS}/print-configuration.cmake")

add_subproject("shared")
add_subproject("efi-to-kernel")
add_subproject("uefi")
add_subproject("efi")
add_platform_abstraction_and_correct_implementations()

add_executable(
    ${PROJECT_NAME}
    "src/main.c"
    "src/cpu.S"
    "src/printing.c"
    "src/gdt.c"
    "src/data-reading.c"
    "src/memory/boot-functions.c"
    "src/memory/boot-functions.c"
    "src/acpi/c-acpi-rsdp.c"
)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

# TODO: Need to change this once multi-platform UEFI support?
target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-cpu)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE uefi-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-i)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-globals)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
