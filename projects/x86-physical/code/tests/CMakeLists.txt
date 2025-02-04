project(x86-physical-tests LANGUAGES C)
# FIXME: MAKE THIS WORK AGAIN

add_executable(${PROJECT_NAME} "src/main.c" "src/physical.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-cpu)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-management-status
)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-management
)

target_link_libraries(${PROJECT_NAME} PRIVATE posix-i)
target_link_libraries(${PROJECT_NAME} PRIVATE posix-log)
target_link_libraries(${PROJECT_NAME} PRIVATE posix-test-framework)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-cpu-status-core)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-cpu-status-test)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
