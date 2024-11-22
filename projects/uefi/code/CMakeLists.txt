cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(uefi LANGUAGES C ASM)
include("../../setup.cmake")

# For some reason, clang-19 does not accept the full file path in --ld-path, so
# we do it like this
get_filename_component(LINKER_FILENAME ${CMAKE_LINKER} NAME)
add_link_options("-fuse-ld=${LINKER_FILENAME}")

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -target x86_64-unknown-windows -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc -fshort-wchar -mno-red-zone"
)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

include("../../print-configuration.cmake")

add_executable(
    ${PROJECT_NAME}
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
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        -Wl,-entry:efi_main,-subsystem:efi_application
        -ffreestanding
        -nostdlib
        -nostdinc
        -target
        x86_64-unknown-windows
)
