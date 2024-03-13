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

set(CMAKE_C_FLAGS "-m64 -fpic -ffreestanding -fno-stack-protector -nostdinc -nostdlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wvla -W")

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

project(os LANGUAGES ASM)

message(STATUS "=== Configuration Settings ===")
message(STATUS "Build type:             ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler:             ${CMAKE_C_COMPILER}")
message(STATUS "C flags:                ${CMAKE_C_FLAGS}")
message(STATUS "Compile Definitions:    ${compile_definitions}")
message(STATUS "Assembler:              ${CMAKE_ASM_COMPILER}")
message(STATUS "Assembler Include:      ${CMAKE_ASM_INCLUDE}")
message(STATUS "Linker:                 ${CMAKE_LINKER}")
message(STATUS "Stripper:               ${CMAKE_STRIPPER}")
message(STATUS "Stripper output:        ${CMAKE_STRIPPER_OUTPUT}")
get_directory_property(compile_definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
message(STATUS "=== End Configuration ===")

set(OS_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/os.img)
add_custom_command(OUTPUT ${OS_OUTPUT}
    COMMAND dd if=/dev/zero of=${OS_OUTPUT} bs=1M count=10 status=none
    VERBATIM
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Creating empty os.img"
    )
add_custom_target(base-image ALL DEPENDS ${OS_OUTPUT})

add_subdirectory(bootloaders)
add_subdirectory(kernel)

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

