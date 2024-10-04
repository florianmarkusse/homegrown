project(memory-management-tests LANGUAGES C)

list(APPEND TEST_SOURCE_FILES "src/main.c" "src/physical.c")

add_executable(${PROJECT_NAME} ${TEST_SOURCE_FILES})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC memory-management)
target_link_libraries(${PROJECT_NAME} PUBLIC status-idt)
target_link_libraries(${PROJECT_NAME} PUBLIC text)
target_link_libraries(${PROJECT_NAME} PUBLIC log)
target_link_libraries(${PROJECT_NAME} PUBLIC test-framework)
