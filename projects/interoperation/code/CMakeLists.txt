cmake_minimum_required(VERSION 3.28)
project(interoperation LANGUAGES C)

include("../../setup.cmake")
initial_setup()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-pic -fno-pie -mcmodel=kernel -nostdinc -nostdlib -m64 -ffreestanding -fno-stack-protector -mno-sse -mno-sse2")

add_library(${MAIN_RESULT} STATIC
    "src/acpi/c-acpi-rsdp.c"
    "src/memory/standard.c"
)

target_include_directories(${MAIN_RESULT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
