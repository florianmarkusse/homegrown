cmake_minimum_required(VERSION 3.28)
project(uefi-image-creator LANGUAGES C ASM)
    
include("../../setup.cmake")
initial_setup()

add_subdirectory(util)

add_executable(${MAIN_RESULT} 
    "src/main.c"
    )

target_include_directories(${MAIN_RESULT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${MAIN_RESULT} PRIVATE util-${CMAKE_BUILD_TYPE})

