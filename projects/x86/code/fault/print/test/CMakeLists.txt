project(x86-fault-print-test LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/test.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts)
