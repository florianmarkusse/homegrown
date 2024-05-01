cmake_minimum_required(VERSION 3.18)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(uefi LANGUAGES C ASM)



include("../../setup.cmake")
initial_setup()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target x86_64-unknown-windows -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc -fshort-wchar -mno-red-zone")

add_executable(${EXECUTABLE_NAME} 
    "src/main.c"
    "src/globals.c"
    "src/printing.c"
    "src/gdt.c"
    "src/data-reading.c"
    "src/memory/standard.c"
    "src/memory/boot-functions.c"
    "src/acpi/c-acpi-rsdp.c"
    "src/acpi/c-acpi-rsdt.c"
    )

target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE "-target x86_64-unknown-windows" "-ffreestanding" "-nostdlib" "-nostdinc" "-Wl,-entry:efi_main" "-Wl,-subsystem:efi_application" "-fuse-ld=lld-link")
