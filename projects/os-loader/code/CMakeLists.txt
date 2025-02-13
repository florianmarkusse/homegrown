cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(os-loader LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)
include("${REPO_PROJECTS}/setup.cmake")
include("${REPO_PROJECTS}/print-configuration.cmake")

add_platform_abstraction_and_correct_implementations()
add_subproject("shared")
add_subproject("efi")
add_subproject("efi-to-kernel")
add_subproject("uefi")

add_executable(
    ${PROJECT_NAME}
    "src/main.c"
    "src/data-reading.c"
    "src/memory/boot-functions.c"
)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-log)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-cpu)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-efi)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)

target_link_libraries(${PROJECT_NAME} PRIVATE uefi-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-i)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-memory)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-globals)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-acpi)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-error)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
