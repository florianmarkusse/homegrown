project(memory-management-tests LANGUAGES C)

list(APPEND TEST_SOURCE_FILES "src/main.c" "src/physical.c")

set(EXECUTABLE_NAME "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_executable(${EXECUTABLE_NAME} ${TEST_SOURCE_FILES})

target_include_directories(
    ${EXECUTABLE_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    ${EXECUTABLE_NAME}
    PUBLIC memory-management-${CMAKE_BUILD_TYPE}
)
target_link_libraries(${EXECUTABLE_NAME} PUBLIC idt-status-${CMAKE_BUILD_TYPE})
target_link_libraries(${EXECUTABLE_NAME} PUBLIC text-${CMAKE_BUILD_TYPE})
target_link_libraries(${EXECUTABLE_NAME} PUBLIC log-${CMAKE_BUILD_TYPE})
target_link_libraries(
    ${EXECUTABLE_NAME}
    PUBLIC test-framework-${CMAKE_BUILD_TYPE}
)
