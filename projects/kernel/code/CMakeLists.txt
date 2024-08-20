cmake_minimum_required(VERSION 3.28)
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
add_library(${MAIN_RESULT} OBJECT "src/main.c" "src/acpi/signature.c")

if("${UNIT_TEST_BUILD}")
    add_compile_definitions(UNIT_TEST_BUILD)
    add_subdirectory("test-framework")
endif()

# Unused directories for now
# add_subdirectory("hash")
# add_subdirectory("buffer")

target_include_directories(
    ${MAIN_RESULT}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_include_directories(
    ${MAIN_RESULT}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)

# Adds the linking library location in reverse order. Remember the linker tries
# to link from last to first argument. So be wise :) This is only for folders
# that are used inside the kernel. For other builds, e.g., unit tests, CMake
# can figure it out.
set(LINKING_LIBRARIES_LOCATIONS)

add_subdirectory("hardware")
target_link_libraries(${MAIN_RESULT} PRIVATE hardware-${CMAKE_BUILD_TYPE})
list(
    APPEND
    LINKING_LIBRARIES_LOCATIONS
    ${CMAKE_CURRENT_BINARY_DIR}/hardware/libhardware-${CMAKE_BUILD_TYPE}.a
)

add_subdirectory("memory")
target_link_libraries(
    ${MAIN_RESULT}
    PRIVATE memory-manipulation-${CMAKE_BUILD_TYPE}
)
list(
    APPEND
    LINKING_LIBRARIES_LOCATIONS
    ${CMAKE_CURRENT_BINARY_DIR}/memory/manipulation/libmemory-manipulation-${CMAKE_BUILD_TYPE}.a
)
target_link_libraries(
    ${MAIN_RESULT}
    PRIVATE memory-management-${CMAKE_BUILD_TYPE}
)
list(
    APPEND
    LINKING_LIBRARIES_LOCATIONS
    ${CMAKE_CURRENT_BINARY_DIR}/memory/management/libmemory-management-${CMAKE_BUILD_TYPE}.a
)

add_subdirectory("peripheral")
target_link_libraries(
    ${MAIN_RESULT}
    PRIVATE peripheral-screen-${CMAKE_BUILD_TYPE}
)
list(
    APPEND
    LINKING_LIBRARIES_LOCATIONS
    ${CMAKE_CURRENT_BINARY_DIR}/peripheral/screen/libperipheral-screen-${CMAKE_BUILD_TYPE}.a
)

add_subdirectory("quickie")
target_link_libraries(${MAIN_RESULT} PRIVATE quickie-${CMAKE_BUILD_TYPE})
list(
    APPEND
    LINKING_LIBRARIES_LOCATIONS
    ${CMAKE_CURRENT_BINARY_DIR}/quickie/libquickie-${CMAKE_BUILD_TYPE}.a
)

add_subdirectory("log")
target_link_libraries(${MAIN_RESULT} PRIVATE log-${CMAKE_BUILD_TYPE})
list(
    APPEND
    LINKING_LIBRARIES_LOCATIONS
    ${CMAKE_CURRENT_BINARY_DIR}/log/liblog-${CMAKE_BUILD_TYPE}.a
)

add_subdirectory("text")
target_link_libraries(${MAIN_RESULT} PRIVATE text-${CMAKE_BUILD_TYPE})
list(
    APPEND
    LINKING_LIBRARIES_LOCATIONS
    ${CMAKE_CURRENT_BINARY_DIR}/text/libtext-${CMAKE_BUILD_TYPE}.a
)

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

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT}
    COMMAND
        ${CMAKE_LINKER} -n -m elf_x86_64 -T ${CMAKE_CURRENT_SOURCE_DIR}/main.ld
        -o ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT}
        $<TARGET_OBJECTS:${MAIN_RESULT}> ${RESOURCE_OBJECTS}
        ${LINKING_LIBRARIES_LOCATIONS}
    COMMAND
        objdump -M x86_64 -D ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT} >
        ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT}.asm
    COMMAND
        objdump -x ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT} >
        ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT}.objdump
    COMMAND_EXPAND_LISTS
    VERBATIM
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT}.asm
        ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT}.objdump
    DEPENDS
        $<TARGET_OBJECTS:${MAIN_RESULT}>
        ${CMAKE_CURRENT_SOURCE_DIR}/main.ld
        ${LINKING_LIBRARIES_LOCATIONS}
        ${RESOURCE_OBJECTS}
    COMMENT "Linking ${MAIN_RESULT}"
)

set(MAIN_RESULT_BIN "${MAIN_RESULT}.bin")
set(KERNEL_MAGIC_OUTPUT
    "${REPO_ROOT}/projects/interoperation/code/include/interoperation/generated/kernel-magic.h"
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT_BIN}
    COMMAND
        objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT}
        ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT_BIN}
    COMMAND
        python "${REPO_ROOT}/projects/interoperation/code/create_magic.py"
        "${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT_BIN}" ${KERNEL_MAGIC_OUTPUT}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT}
    COMMAND_EXPAND_LISTS
    BYPRODUCTS ${KERNEL_MAGIC_OUTPUT}
    VERBATIM
    COMMENT "Copying binary data from ${MAIN_RESULT} into ${MAIN_RESULT_BIN}"
)
add_custom_target(
    "${MAIN_RESULT}-binary-creator"
    ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_RESULT_BIN}
)
