cmake_minimum_required(VERSION 3.18)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(uefi LANGUAGES C ASM)

include("../../setup.cmake")
initial_setup()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target x86_64-unknown-windows -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc -fshort-wchar -mno-red-zone")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

add_executable(${EXECUTABLE_NAME} 
    #  "src/bootboot.c"
    #  "src/smp.S"
      "src/main.c"
    "src/globals.c"
    "src/printing.c"
    "src/gdt.c"
    "src/data-reading.c"
    "src/memory/boot-functions.c"
    )

target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../interoperation/code" "${CMAKE_CURRENT_SOURCE_DIR}/../../interoperation/code/build")
target_link_libraries(${EXECUTABLE_NAME} PRIVATE interoperation-${CMAKE_BUILD_TYPE})

target_link_libraries(${EXECUTABLE_NAME} PRIVATE "-target x86_64-unknown-windows" "-ffreestanding" "-nostdlib" "-nostdinc" "-Wl,-entry:efi_main" "-Wl,-subsystem:efi_application" "-fuse-ld=lld-link")
