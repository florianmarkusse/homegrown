project(memory-management-tests LANGUAGES C)

list(APPEND TEST_SOURCE_FILES "src/main.c")

set(EXECUTABLE_NAME "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_executable(${EXECUTABLE_NAME} ${TEST_SOURCE_FILES})

target_link_libraries(
    ${EXECUTABLE_NAME}
    PUBLIC memory-management-${CMAKE_BUILD_TYPE}
)
target_link_libraries(${EXECUTABLE_NAME} PUBLIC text-${CMAKE_BUILD_TYPE})
target_link_libraries(${EXECUTABLE_NAME} PUBLIC log-${CMAKE_BUILD_TYPE})
target_link_libraries(
    ${EXECUTABLE_NAME}
    PUBLIC test-framework-${CMAKE_BUILD_TYPE}
)
