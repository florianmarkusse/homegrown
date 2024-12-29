cmake_minimum_required(VERSION 3.22)
project(uefi-i LANGUAGES C ASM)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    add_link_options("--ld-path=${CMAKE_LINKER}")
    add_subproject("shared")
endif()

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} INTERFACE shared-i)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
