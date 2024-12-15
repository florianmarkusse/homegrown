cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(uefi LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)
include("${REPO_PROJECTS}/setup.cmake")

# For some reason, clang-19 does not accept the full file path in --ld-path, so
# we do it like this
get_filename_component(LINKER_FILENAME ${CMAKE_LINKER} NAME)
add_link_options(
    -fuse-ld=${LINKER_FILENAME}
    -Wl,-entry:efi_main,-subsystem:efi_application
    -ffreestanding
    -nostdlib
    -nostdinc
    -target
    x86_64-unknown-windows
)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -target x86_64-unknown-windows -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc -fshort-wchar -mno-red-zone"
)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

include("${REPO_PROJECTS}/print-configuration.cmake")

add_subproject("shared")
add_subproject("interoperation")
add_platform_abstraction_and_correct_implementations()

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
    "src/acpi/c-acpi-rsdp.c"
)

target_link_libraries(${PROJECT_NAME} PRIVATE interoperation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-cpu)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
