cmake_minimum_required(VERSION 3.28)
project(kernel-test LANGUAGES C ASM)
include("../../setup.cmake")

add_custom_target(${MAIN_RESULT} ALL)

include(ExternalProject)

set(TARGET_PREFIX "kernel-test")

set(TEST_OBJECT "test-object")
ExternalProject_Add(
    ${TEST_OBJECT}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/${TEST_OBJECT}
    SOURCE_DIR ${REPO_PROJECTS}/kernel/code
    CMAKE_ARGS
        -DUNIT_TEST=ON -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_LINKER=${CMAKE_LINKER} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DUSE_AVX=${USE_AVX} -DUSE_SSE=${USE_SSE}
    INSTALL_COMMAND
        "" # Disable install step
    LOG_CONFIGURE
        ON # Enable verbose logging for configure step
    LOG_BUILD
        ON # Enable verbose logging for build step
)

# Get the binary directory and define the location of the library to be imported
ExternalProject_Get_Property(${TEST_OBJECT} BINARY_DIR)
set(IMPORTED_LIB_LOCATION "${BINARY_DIR}/libunit-test-kernel.a") # Adjust the library name as needed

set(KERNEL_LIBRARY "kernel-library")
# Create the imported library target
add_library(${KERNEL_LIBRARY} STATIC IMPORTED)
set_target_properties(
    ${KERNEL_LIBRARY}
    PROPERTIES IMPORTED_LOCATION "${IMPORTED_LIB_LOCATION}"
)

# Ensure the external project builds before tests
add_dependencies(${KERNEL_LIBRARY} ${TEST_OBJECT})
add_dependencies(${MAIN_RESULT} ${KERNEL_LIBRARY})

add_subdirectory(util)
add_subdirectory(test-framework)

add_subdirectory(tests)
