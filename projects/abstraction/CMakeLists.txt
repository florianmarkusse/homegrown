cmake_minimum_required(VERSION 3.22)
project(abstraction LANGUAGES C)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
endif()

add_subdirectory("efi/code")
add_subdirectory("interrupts/code")
add_subdirectory("log/code")

add_subdirectory("memory/manipulation/code")
add_subdirectory("memory/virtual/code")

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
