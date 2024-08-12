project(memory-management-tests LANGUAGES C)

list(APPEND TEST_SOURCE_FILES
    "src/main.c"
)

set(TESTS "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_executable(${TESTS} ${TEST_SOURCE_FILES})

target_include_directories(${TESTS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
