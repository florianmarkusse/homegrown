cmake_minimum_required(VERSION 3.22)
project(platform-abstraction LANGUAGES C)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("../../setup.cmake")
    include("../../print-configuration.cmake")
endif()

add_subdirectory(log)
