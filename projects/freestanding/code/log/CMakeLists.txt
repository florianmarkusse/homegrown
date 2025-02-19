project(freestanding-log LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/log.c")

add_includes_for_sublibrary()

add_subdirectory(init)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE freestanding-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
