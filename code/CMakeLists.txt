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

# TODO: remove fpic?
set(CMAKE_C_FLAGS "-fpic -ffreestanding -fno-stack-protector -nostdinc -nostdlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wvla -W")

set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -n -T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld")

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


project(testos LANGUAGES C)

message(STATUS "=== Configuration Settings ===")
message(STATUS "Build type:             ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler:             ${CMAKE_C_COMPILER}")
message(STATUS "C flags:                ${CMAKE_C_FLAGS}")
message(STATUS "Linker:                 ${CMAKE_LINKER}")
message(STATUS "Linker Exe flags:       ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "Linker Shared flags:    ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "sanitize:               ${SANITIZE_OPTIONS}")
get_directory_property(compile_definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
message(STATUS "Compile Definitions:    ${compile_definitions}")
message(STATUS "=== End Configuration ===")

set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
file(GLOB_RECURSE RESOURCE_FILES ${RESOURCE_DIR}/*)
set(RESOURCE_OBJECTS)
foreach(RESOURCE_FILE ${RESOURCE_FILES})
    string(REPLACE ${RESOURCE_DIR}/ "" RESOURCE_RELATIVE_PATH ${RESOURCE_FILE})
    string(REGEX REPLACE "/" "_" RESOURCE_RELATIVE_PATH ${RESOURCE_RELATIVE_PATH})
    string(REGEX REPLACE "\\..*$" "" RESOURCE_RELATIVE_PATH ${RESOURCE_RELATIVE_PATH})
    set(RESOURCE_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_RELATIVE_PATH}.o)
    list(APPEND RESOURCE_OBJECTS ${RESOURCE_OBJECT})


message(STATUS "path:    ${RESOURCE_RELATIVE_PATH}")
    add_custom_command(
        OUTPUT font.o
        # COMMAND ${CMAKE_LINKER} -r -b binary -o ${RESOURCE_OBJECT} ${RESOURCE_FILE}
        COMMAND x86_64-testos-elf-ld -r -b binary -o build/font.o resources/font.psf
        DEPENDS ${RESOURCE_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Embedding ${RESOURCE_FILE} into ${RESOURCE_OBJECT}"
    )
endforeach()

add_custom_target(embedded-resources ALL DEPENDS ${RESOURCE_OBJECTS})

list(APPEND SOURCE_FILES
    "src/kernel.c"
)

set(EXECUTABLE_NAME "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${RESOURCE_OBJECTS})

target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

