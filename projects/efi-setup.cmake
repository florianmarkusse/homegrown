# For some reason, clang-19 does not accept the full file path in --ld-path, so
# we do it like this
get_filename_component(LINKER_FILENAME ${CMAKE_LINKER} NAME)
add_link_options(
    -fuse-ld=${LINKER_FILENAME}
    -Wl,-entry:efi_main,-subsystem:efi_application
)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -target x86_64-unknown-windows -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc -mno-stack-arg-probe"
)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
