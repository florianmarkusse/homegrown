cmake_minimum_required(VERSION 3.18)

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

set(CMAKE_C_FLAGS "-fpic -ffreestanding -fno-stack-protector -nostdinc -nostdlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wvla -W")

set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -n -T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld")


add_compile_definitions("__AVX2__")
if (CMAKE_BUILD_TYPE STREQUAL "Fuzzing" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
    add_compile_definitions("DEBUG")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Profiling")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -O2 -pg")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

project(os LANGUAGES ASM)

message(STATUS "=== Configuration Settings ===")
message(STATUS "Build type:             ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler:             ${CMAKE_C_COMPILER}")
message(STATUS "C flags:                ${CMAKE_C_FLAGS}")
message(STATUS "Compile Definitions:    ${compile_definitions}")
message(STATUS "Assembler:              ${CMAKE_ASM_COMPILER}")
message(STATUS "Assembler Include:      ${CMAKE_ASM_INCLUDE}")
message(STATUS "Linker:                 ${CMAKE_LINKER}")
message(STATUS "Linker Exe flags:       ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "Linker Shared flags:    ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "Stripper:               ${CMAKE_STRIPPER}")
message(STATUS "Stripper output:        ${CMAKE_STRIPPER_OUTPUT}")
get_directory_property(compile_definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
message(STATUS "=== End Configuration ===")

# set(RESOURCES_NAME "resources")
# file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCES_NAME})
# set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCES_NAME})
# file(GLOB_RECURSE RESOURCE_FILES ${RESOURCE_DIR}/*)
# set(RESOURCE_OBJECTS)
# foreach(RESOURCE_FILE ${RESOURCE_FILES})
#     string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/ "" RESOURCE_RELATIVE_PATH ${RESOURCE_FILE})
#     string(REGEX REPLACE "\\..*$" "" RESOURCE_OUTPUT_PATH ${RESOURCE_RELATIVE_PATH})
#     set(RESOURCE_OUTPUT_PATH ${RESOURCE_OUTPUT_PATH}.o)
# 
#     set(RESOURCE_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_OUTPUT_PATH})
#     list(APPEND RESOURCE_OBJECTS ${RESOURCE_OBJECT})
# 
#     add_custom_command(
#         OUTPUT ${RESOURCE_OUTPUT_PATH}
#         COMMAND ${CMAKE_LINKER} -r -b binary -o ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_OUTPUT_PATH} ${RESOURCE_RELATIVE_PATH}
#         DEPENDS ${RESOURCE_FILE}
#         VERBATIM
#         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#         COMMENT "Embedding ${RESOURCE_FILE} into ${RESOURCE_OBJECT}"
#     )
# endforeach()
# 
# add_custom_target(embedded-resources ALL DEPENDS ${RESOURCE_OBJECTS})


set(BOOTLOADER_OUTPUT bootloader.iso)
message(STATUS "ghdkjfhgkfd output:        ${BOOTLOADER_OUTPUT}")
set(BOOTLOADER_FILE "src/bootloader.asm")
string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/ "" BOOTLOADER_RELATIVE_PATH ${BOOTLOADER_FILE})
add_custom_command(
    OUTPUT ${BOOTLOADER_OUTPUT}
    COMMAND INCLUDE=${CMAKE_ASM_INCLUDE} ${CMAKE_ASM_COMPILER} -e 100 ${BOOTLOADER_RELATIVE_PATH} ${CMAKE_CURRENT_BINARY_DIR}/bootloader.iso
    DEPENDS ${BOOTLOADER_FILE}
    VERBATIM
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Assembling ${BOOTLOADER_FILE} into ${BOOTLOADER_OUTPUT}"
)
add_custom_target(bootloader ALL DEPENDS ${BOOTLOADER_OUTPUT})


# set(EXECUTABLE_NAME "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
# 
# add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})
# 
# add_subdirectory(util)
# 
# target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${RESOURCE_OBJECTS})
# target_link_libraries(${EXECUTABLE_NAME} PRIVATE util-${CMAKE_BUILD_TYPE})
# 
# target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
