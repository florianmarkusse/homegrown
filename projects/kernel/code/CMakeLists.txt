cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(kernel LANGUAGES C ASM)
include("../../setup.cmake")

if(NOT "${UNIT_TEST_BUILD}")
    set(CMAKE_C_FLAGS
        # TODO: SSE is still broken!!!
        "${CMAKE_C_FLAGS} -fno-pic -fno-pie -mcmodel=kernel -nostdinc -nostdlib -m64 -ffreestanding -fno-stack-protector -mno-sse -mno-sse2"
    )
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} --embed-dir=${REPO_PROJECTS}/kernel/resources"
    )
endif()

include("../../print-configuration.cmake")

add_library(${PROJECT_NAME} OBJECT "src/main.c")

if("${UNIT_TEST_BUILD}")
    add_subdirectory("test-framework")
endif()

# Unused directories for now
# add_subdirectory("hash")
# add_subdirectory("buffer")

add_subdirectory("cpu")
add_subdirectory("memory")
add_subdirectory("peripheral")
add_subdirectory("log")
add_subdirectory("text")
add_subdirectory("util")
add_subdirectory("status")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

# Adds the linking library location in reverse order. Remember the linker tries
# to link from last to first argument. So be wise :) This is only for folders
# that are used inside the kernel. For other builds, e.g., unit tests, CMake
# can figure it out.
set(LINKING_LIBRARIES_LOCATIONS)

function(add_to_linker name)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${name})
    string(REPLACE "-" "/" slashenated "${name}")
    set(lib_path "${CMAKE_CURRENT_BINARY_DIR}/${slashenated}/lib${name}.a")
    list(APPEND LINKING_LIBRARIES_LOCATIONS ${lib_path})
    set(LINKING_LIBRARIES_LOCATIONS ${LINKING_LIBRARIES_LOCATIONS} PARENT_SCOPE)
endfunction()

# NOTE: Should add some debug or whatever option that adds all the status libs
add_to_linker("status-memory")

add_to_linker("log")
add_to_linker("text")
add_to_linker("peripheral-screen")
add_to_linker("memory-management")
add_to_linker("util")
add_to_linker("cpu")
add_to_linker("memory-manipulation")

if("${UNIT_TEST_BUILD}")
    return()
endif()

set(LINKER_TARGET "${PROJECT_NAME}-linker")
add_custom_target(
    ${LINKER_TARGET}
    ALL
    COMMAND
        ${CMAKE_LINKER} -n -m elf_x86_64 -T ${CMAKE_CURRENT_SOURCE_DIR}/main.ld
        -o ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        $<TARGET_OBJECTS:${PROJECT_NAME}> ${RESOURCE_OBJECTS}
        ${LINKING_LIBRARIES_LOCATIONS}
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
