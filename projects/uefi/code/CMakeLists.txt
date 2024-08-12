cmake_minimum_required(VERSION 3.28)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(uefi LANGUAGES C ASM)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -target x86_64-unknown-windows -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc -fshort-wchar -mno-red-zone"
)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

include("../../setup.cmake")
initial_setup()

add_executable(
    ${MAIN_RESULT}
    #  "src/bootboot.c"
    #  "src/smp.S"
    "src/main.c"
    "src/cpu.S"
    "src/globals.c"
    "src/printing.c"
    "src/gdt.c"
    "src/data-reading.c"
    "src/memory/boot-functions.c"
    "src/memory/boot-functions.c"
    "src/memory/standard.c"
    "src/acpi/c-acpi-rsdp.c"
)

target_include_directories(
    ${MAIN_RESULT}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    ${MAIN_RESULT}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)

target_link_libraries(
    ${MAIN_RESULT}
    PRIVATE
        "-target x86_64-unknown-windows"
        "-ffreestanding"
        "-nostdlib"
        "-nostdinc"
        "-Wl,-entry:efi_main"
        "-Wl,-subsystem:efi_application"
        "-fuse-ld=lld-link"
)
