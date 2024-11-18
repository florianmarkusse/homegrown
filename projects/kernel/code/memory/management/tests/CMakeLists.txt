project(memory-management-tests LANGUAGES C)

add_executable(${PROJECT_NAME} "src/main.c" "src/physical.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-log)

target_link_libraries(${PROJECT_NAME} PRIVATE memory-management)
target_link_libraries(${PROJECT_NAME} PRIVATE status-idt)
target_link_libraries(${PROJECT_NAME} PRIVATE cpu)

target_link_libraries(${PROJECT_NAME} PRIVATE posix-test-framework)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
