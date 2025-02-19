project(freestanding-memory-manipulation LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/manipulation.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
