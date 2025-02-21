project(x86-idt LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/trigger.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-interrupts-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

add_subdirectory(real)
add_subdirectory(mock)
