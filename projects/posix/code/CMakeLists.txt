cmake_minimum_required(VERSION 3.22)
project(posix LANGUAGES C ASM)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("../../setup.cmake")
    include("../../print-configuration.cmake")
    add_subdirectory(
        "${REPO_PROJECTS}/shared/code"
        "${CMAKE_CURRENT_BINARY_DIR}/shared"
    )
endif()

add_subdirectory(log)
add_subdirectory(file)
