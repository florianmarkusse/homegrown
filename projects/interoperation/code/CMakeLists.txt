cmake_minimum_required(VERSION 3.22)
project(interoperation LANGUAGES C ASM)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    add_link_options("--ld-path=${CMAKE_LINKER}")
    add_platform_abstraction_and_correct_implementations()
    add_subproject("shared")
    add_subproject("x86")
endif()

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} INTERFACE shared-types)
target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE platform-abstraction-memory-manipulation
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
