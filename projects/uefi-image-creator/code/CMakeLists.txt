cmake_minimum_required(VERSION 3.18)
project(uefi-image-creator LANGUAGES C ASM)
    
include("../../setup.cmake")
initial_setup()

add_subdirectory(util)

add_executable(${EXECUTABLE_NAME} 
    "src/main.c"
    )

target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE util-${CMAKE_BUILD_TYPE})

