cmake_minimum_required(VERSION 3.28)
project(kernel-test LANGUAGES C ASM)

include("../../setup.cmake")
initial_setup()

add_subdirectory(util)
add_subdirectory(test-framework)

add_subdirectory(tests)

# target_link_libraries(${MAIN_RESULT} PRIVATE util-${CMAKE_BUILD_TYPE})
