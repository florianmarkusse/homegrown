project(x86-memory-physical-tests LANGUAGES C)
add_executable(${PROJECT_NAME} "src/main.c" "src/physical.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE posix-i)
target_link_libraries(${PROJECT_NAME} PRIVATE posix-test-framework)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-idt-mock)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-fault-print)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-fault-print-test)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory-physical)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
