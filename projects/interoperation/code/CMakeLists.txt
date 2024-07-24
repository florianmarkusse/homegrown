cmake_minimum_required(VERSION 3.28)
project(interoperation LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-pic -fno-pie -nostdinc -nostdlib -m64 -ffreestanding -fno-stack-protector")

include("../../setup.cmake")
initial_setup()

add_executable(${MAIN_RESULT} 
    "src/main.c"
    )

target_include_directories(${MAIN_RESULT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
