cmake_minimum_required(VERSION 3.18)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug, Release, Profiling, Fuzzing)" FORCE)
endif()
set(VALID_BUILD_TYPES "Debug" "Release" "Profiling" "Fuzzing")
list(FIND VALID_BUILD_TYPES ${CMAKE_BUILD_TYPE} VALID_BUILD_TYPE_INDEX)
if(VALID_BUILD_TYPE_INDEX EQUAL -1)
    message(FATAL_ERROR "Invalid build type specified. Please choose one of: ${VALID_BUILD_TYPES}")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wvla -W")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -nostdlib -m64 -fpic -ffreestanding -fno-stack-protector")

if (CMAKE_BUILD_TYPE STREQUAL "Fuzzing" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
    add_compile_definitions("DEBUG")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Profiling")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -O2 -pg")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(REPO_DEPENDENCIES ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies)

project(kernel LANGUAGES C)

message(STATUS "=== Configuration Settings ===")
message(STATUS "Build type:             ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler:             ${CMAKE_C_COMPILER}")
message(STATUS "C flags:                ${CMAKE_C_FLAGS}")
message(STATUS "Compile Definitions:    ${compile_definitions}")
message(STATUS "Assembler:              ${CMAKE_ASM_COMPILER}")
message(STATUS "Assembler Include:      ${CMAKE_ASM_INCLUDE}")
message(STATUS "Linker:                 ${CMAKE_LINKER}")
message(STATUS "repo root:              ${REPO_ROOT}")
message(STATUS "dependencies:           ${REPO_DEPENDENCIES}")
get_directory_property(compile_definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
message(STATUS "=== End Configuration ===")

set(EXECUTABLE_NAME "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_library(${EXECUTABLE_NAME} OBJECT
        "kernel.c" 
        )

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}
        COMMAND ${CMAKE_LINKER} -m elf_x86_64 -T ${CMAKE_CURRENT_SOURCE_DIR}/kernel.ld -o ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME} $<TARGET_OBJECTS:${EXECUTABLE_NAME}>
        COMMAND objdump -M x86_64 -D ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME} > ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.asm
        COMMAND objdump -x ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME} > ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.objdump
        COMMAND_EXPAND_LISTS
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.asm ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.objdump
        DEPENDS $<TARGET_OBJECTS:${EXECUTABLE_NAME}> kernel ${CMAKE_CURRENT_SOURCE_DIR}/kernel.ld
        COMMENT "Linking ${EXECUTABLE_NAME}"
)

set(EXECUTABLE_NAME_BIN "${EXECUTABLE_NAME}.bin")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME_BIN}
        COMMAND objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME_BIN}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}
        COMMENT "Copying binary data from ${EXECUTABLE_NAME} into ${EXECUTABLE_NAME_BIN}"
)

add_custom_target("${EXECUTABLE_NAME}-binary" ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME_BIN})
