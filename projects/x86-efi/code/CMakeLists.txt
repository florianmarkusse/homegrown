cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(x86-efi LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(${REPO_PROJECTS}/setup.cmake)
    include(${REPO_PROJECTS}/print-configuration.cmake)
    add_subproject("shared")
    add_subproject("efi-to-kernel")
    add_subproject("uefi")
    add_subproject("efi")
    add_platform_abstraction_and_correct_implementations()
endif()

add_library(${PROJECT_NAME} OBJECT "src/init.c")

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-log)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
