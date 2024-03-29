macro(initial_setup)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wvla -W")
    
    set(EXECUTABLE_NAME "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
    
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
    
    set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    set(REPO_DEPENDENCIES ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies)
    
    message(STATUS "=== Configuration Settings ===")
    message(STATUS "Project:                ${PROJECT_NAME}")
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
endmacro()