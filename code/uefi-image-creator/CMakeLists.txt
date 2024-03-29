cmake_minimum_required(VERSION 3.18)
project(uefi-image-creator LANGUAGES C ASM)
    
include("../setup.cmake")
initial_setup()

add_subdirectory(util)

add_executable(${EXECUTABLE_NAME} 
    "write_gpt.c"
    )

target_link_libraries(${EXECUTABLE_NAME} PRIVATE util-${CMAKE_BUILD_TYPE})

