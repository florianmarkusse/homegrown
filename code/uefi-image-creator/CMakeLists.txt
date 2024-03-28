cmake_minimum_required(VERSION 3.18)
project(uefi-image-creator LANGUAGES C ASM)

include("../setup.cmake")
initial_setup()

add_executable(${EXECUTABLE_NAME} 
    "write_gpt.c"
    )
