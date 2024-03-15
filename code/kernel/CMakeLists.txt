project(kernel LANGUAGES C)


set(KERNEL_NAME "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_library(${KERNEL_NAME} OBJECT
        "kernel.c" 
        )

set(KERNEL_LINKED "${KERNEL_NAME}-linked")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_LINKED}
        COMMAND ${CMAKE_LINKER} -m elf_x86_64 -T ${CMAKE_CURRENT_SOURCE_DIR}/kernel.ld -o ${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_LINKED} $<TARGET_OBJECTS:${KERNEL_NAME}>
        COMMAND objdump -M x86_64 -D ${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_LINKED} > ${CMAKE_CURRENT_BINARY_DIR}/kernel.asm
        COMMAND objdump -x ${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_LINKED} > ${CMAKE_CURRENT_BINARY_DIR}/kernel.objdump
        COMMAND_EXPAND_LISTS
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/kernel.asm ${CMAKE_CURRENT_BINARY_DIR}/kernel.objdump
        DEPENDS $<TARGET_OBJECTS:${KERNEL_NAME}> kernel ${CMAKE_CURRENT_SOURCE_DIR}/kernel.ld
        COMMENT "compiling and linking ${KERNEL_NAME}"
)


add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
        COMMAND objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_LINKED} ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
        COMMAND dd if=${CMAKE_CURRENT_BINARY_DIR}/kernel.bin of=${OS_OUTPUT} bs=512 seek=64 count=256 conv=notrunc
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_LINKED}
        COMMENT "copying ${KERNEL_LINKED} and writing into ${OS_OUTPUT}"
)

add_custom_target(kernel-binary ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin)
