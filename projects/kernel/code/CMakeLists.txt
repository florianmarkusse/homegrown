cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(kernel LANGUAGES C ASM)
include("../../setup.cmake")

if(NOT "${UNIT_TEST_BUILD}")
    set(CMAKE_C_FLAGS
        # TODO: SSE is still broken!!!
        "${CMAKE_C_FLAGS} -fno-pic -fno-pie -mcmodel=kernel -m64 -fno-stack-protector -mno-sse -mno-sse2"
    )
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
endif()

# This is a C-only flag. Don't add this to the ASM flags
set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} --embed-dir=${REPO_PROJECTS}/kernel/resources"
)

include("../../print-configuration.cmake")

add_subdirectory(
    "${REPO_PROJECTS}/shared/code"
    "${CMAKE_CURRENT_BINARY_DIR}/shared"
)

add_subdirectory("${REPO_PROJECTS}/x86/code" "${CMAKE_CURRENT_BINARY_DIR}/x86")

if("${UNIT_TEST_BUILD}")
    add_subdirectory(
        "${REPO_PROJECTS}/posix/code"
        "${CMAKE_CURRENT_BINARY_DIR}/posix"
    )
    return()
endif()

add_subdirectory("cpu")
add_subdirectory("memory")
add_subdirectory("status")

if("${UNIT_TEST_BUILD}")
    return()
endif()

################################################################################
#       End of common setup, below is only for production kernel build         #
################################################################################

add_subdirectory("log")
add_subdirectory("peripheral")

add_library(${PROJECT_NAME} OBJECT "src/main.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE cpu)
target_link_libraries(${PROJECT_NAME} PRIVATE log)
target_link_libraries(${PROJECT_NAME} PRIVATE status-memory)

# NOTE: Should add some debug or whatever option that adds all the status libs
set(OBJECT_TARGETS
    # Shared
    $<TARGET_OBJECTS:shared-text>
    $<TARGET_OBJECTS:shared-maths>
    $<TARGET_OBJECTS:shared-memory-manipulation>
    $<TARGET_OBJECTS:shared-memory-allocator>
    # Architecture-specific
    $<TARGET_OBJECTS:x86-memory>
    # Kernel
    $<TARGET_OBJECTS:memory-management>
    $<TARGET_OBJECTS:log>
    $<TARGET_OBJECTS:peripheral-screen>
    $<TARGET_OBJECTS:cpu>
    # Kernel Status
    $<TARGET_OBJECTS:status-memory>
)

set(LINKER_TARGET "${PROJECT_NAME}-linker")
add_custom_target(
    ${LINKER_TARGET}
    ALL
    COMMAND
        ${CMAKE_LINKER} -n -m elf_x86_64 -T ${CMAKE_CURRENT_SOURCE_DIR}/main.ld
        -o ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        $<TARGET_OBJECTS:${PROJECT_NAME}> ${OBJECT_TARGETS}
    COMMAND
        objdump -M x86_64 -D ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} >
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.asm
    COMMAND
        objdump -x ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} >
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.objdump
    COMMAND_EXPAND_LISTS
    VERBATIM
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.asm
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.objdump
    COMMENT "Linking ${PROJECT_NAME}"
)
add_dependencies(${LINKER_TARGET} ${PROJECT_NAME})

set(BINARIZER "${PROJECT_NAME}-binarizer")
set(PROJECT_NAME_BIN "${PROJECT_NAME}.bin")
set(KERNEL_MAGIC_FOLDER
    "${REPO_PROJECTS}/interoperation/code/include/interoperation/generated"
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
        "${REPO_PROJECTS}/interoperation/code/create_magic.py"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}" ${KERNEL_MAGIC_OUTPUT}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    COMMAND_EXPAND_LISTS
    BYPRODUCTS ${KERNEL_MAGIC_OUTPUT}
    VERBATIM
    COMMENT "Copying binary data from ${PROJECT_NAME} into ${PROJECT_NAME_BIN}"
)
add_dependencies(${BINARIZER} ${LINKER_TARGET})
