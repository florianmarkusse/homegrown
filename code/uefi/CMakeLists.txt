cmake_minimum_required(VERSION 3.18)
project(uefi LANGUAGES C ASM)

include("../setup.cmake")
initial_setup()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target x86_64-unknown-windows -ffreestanding -nostdlib -nostdinc -fshort-wchar -mno-red-zone")

add_executable(${EXECUTABLE_NAME} 
    "hello.c"
    )

target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE "-target x86_64-unknown-windows" "-ffreestanding" "-nostdlib" "-nostdinc" "-Wl,-entry:efi_main" "-Wl,-subsystem:efi_application" "-fuse-ld=lld-link")
