cmake_minimum_required(VERSION 3.22)
project(posix LANGUAGES C ASM)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(${REPO_PROJECTS}/setup.cmake)
    include(${REPO_PROJECTS}/print-configuration.cmake)
    add_link_options(--ld-path=${CMAKE_LINKER})
    add_subproject("shared")
    add_subproject("platform-abstraction")
endif()

add_subdirectory(log)
add_subdirectory(file)
add_subdirectory(test-framework)
