project(quickie-tests LANGUAGES C)

list(APPEND TEST_SOURCE_FILES "src/main.c")

set(EXECUTABLE_NAME "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_executable(${EXECUTABLE_NAME} ${TEST_SOURCE_FILES})

target_link_libraries(${EXECUTABLE_NAME} PUBLIC quickie-${CMAKE_BUILD_TYPE})
