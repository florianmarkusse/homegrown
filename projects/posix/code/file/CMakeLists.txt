project(posix-file LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/path.c" "src/read.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE posix-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
