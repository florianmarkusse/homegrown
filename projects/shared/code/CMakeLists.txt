cmake_minimum_required(VERSION 3.22)
project(shared LANGUAGES C ASM)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("../../setup.cmake")
    include("../../print-configuration.cmake")
endif()

add_subdirectory(text)
add_subdirectory(memory)
