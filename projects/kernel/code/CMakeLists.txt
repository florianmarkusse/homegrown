cmake_minimum_required(VERSION 3.22)
project(kernel LANGUAGES C ASM)

if(NOT "${UNIT_TEST_BUILD}")
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -fno-pic -fno-pie -mcmodel=kernel -nostdinc -nostdlib -m64 -ffreestanding -fno-stack-protector -mno-sse -mno-sse2"
    )
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
endif()

include("../../setup.cmake")

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# TODO: SSE is still broken!!!
add_library(${PROJECT_NAME} OBJECT "src/main.c")

# Needs to be on top, since it takes care of the compile definition!
if("${UNIT_TEST_BUILD}")
    add_compile_definitions(UNIT_TEST_BUILD)
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
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
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

set(RESOURCES_NAME "resources")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCES_NAME})
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${RESOURCES_NAME}")
file(GLOB_RECURSE RESOURCE_FILES ${RESOURCE_DIR}/*)
set(RESOURCE_OBJECTS)
foreach(RESOURCE_FILE ${RESOURCE_FILES})
    string(REPLACE "${RESOURCE_DIR}/" "" RESOURCE_OBJECT ${RESOURCE_FILE})
    string(
        REPLACE
        "${CMAKE_CURRENT_SOURCE_DIR}/"
        ""
        LOCAL_RESOURCE_FILE
        ${RESOURCE_FILE}
    )
    set(RESOURCE_OBJECT "${RESOURCE_OBJECT}.o")

    set(RESOURCE_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/${RESOURCES_NAME}/${RESOURCE_OBJECT}"
    )
    list(APPEND RESOURCE_OBJECTS ${RESOURCE_OBJECT})

    add_custom_command(
        OUTPUT ${RESOURCE_OBJECT}
        COMMAND
            ${CMAKE_LINKER} -r -b binary -o ${RESOURCE_OBJECT}
            ${LOCAL_RESOURCE_FILE}
        DEPENDS ${LOCAL_RESOURCE_FILE}
        VERBATIM
        COMMAND_EXPAND_LISTS
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT
            "Creating binary from ${LOCAL_RESOURCE_FILE} into ${RESOURCE_OBJECT}"
    )
endforeach()
add_custom_target(embedded-resources ALL DEPENDS ${RESOURCE_OBJECTS})

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
add_dependencies(${LINKER_TARGET} embedded-resources ${PROJECT_NAME})

set(BINARIZER "${PROJECT_NAME}-binarizer")
set(PROJECT_NAME_BIN "${PROJECT_NAME}.bin")
set(KERNEL_MAGIC_FOLDER
    "${REPO_ROOT}/projects/interoperation/code/include/interoperation/generated"
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
        "${REPO_ROOT}/projects/interoperation/code/create_magic.py"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}" ${KERNEL_MAGIC_OUTPUT}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    COMMAND_EXPAND_LISTS
    BYPRODUCTS ${KERNEL_MAGIC_OUTPUT}
    VERBATIM
    COMMENT "Copying binary data from ${PROJECT_NAME} into ${PROJECT_NAME_BIN}"
)
add_dependencies(${BINARIZER} ${LINKER_TARGET})
