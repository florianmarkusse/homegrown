cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(kernel LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

set(CMAKE_C_FLAGS
    # TODO: SSE is still broken!!!
    "${CMAKE_C_FLAGS} -fno-pic -fno-pie -mcmodel=kernel -m64 -fno-stack-protector -mno-sse -mno-avx"
)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

include("${REPO_PROJECTS}/setup.cmake")

add_subproject("shared")
add_subproject("efi-to-kernel")
add_subproject("freestanding")
add_subproject("abstraction/log")
add_subproject("abstraction/interrupts")
add_subproject("abstraction/memory/management")
add_subproject("abstraction/memory/manipulation")
add_subproject("abstraction/memory/physical")
add_subproject("abstraction/memory/virtual")

include("${REPO_PROJECTS}/print-configuration.cmake")

add_executable(${PROJECT_NAME} "src/main.c")

if(${ARCHITECTURE} STREQUAL "x86")
    set(EMULATION_TARGET "elf_x86_64")
else()
    message(FATAL "No emulation target set to link")
endif()
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        LINK_FLAGS
            "-T ${CMAKE_CURRENT_SOURCE_DIR}/main.ld -no-pie -Wl,-n,-m,${EMULATION_TARGET},--build-id=none"
        EXCLUDE_FROM_ALL TRUE
)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-virtual-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-physical-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-management)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation)

target_link_libraries(${PROJECT_NAME} PRIVATE freestanding-i)
target_link_libraries(${PROJECT_NAME} PRIVATE freestanding-log-init)
target_link_libraries(${PROJECT_NAME} PRIVATE freestanding-peripheral-screen)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-converter)

set(BINARIZER "${PROJECT_NAME}.bin")
set(PROJECT_NAME_BIN "${PROJECT_NAME}.bin")
set(KERNEL_MAGIC_FOLDER
    "${REPO_PROJECTS}/efi-to-kernel/code/include/efi-to-kernel/generated"
)
set(KERNEL_MAGIC_OUTPUT "${KERNEL_MAGIC_FOLDER}/kernel-magic.h")
add_custom_target(
    ${BINARIZER}
    ALL
    COMMAND
        objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}
    COMMAND
        mkdir -p ${KERNEL_MAGIC_FOLDER} && python3
        "${REPO_PROJECTS}/efi-to-kernel/code/create_magic.py"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}" ${KERNEL_MAGIC_OUTPUT}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    COMMAND_EXPAND_LISTS
    BYPRODUCTS ${KERNEL_MAGIC_OUTPUT}
    VERBATIM
    COMMENT "Transforming ${PROJECT_NAME} into ${PROJECT_NAME_BIN}"
)
add_dependencies(${BINARIZER} ${PROJECT_NAME})

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
