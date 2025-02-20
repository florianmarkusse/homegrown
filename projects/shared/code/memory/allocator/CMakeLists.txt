project(shared-memory-allocator LANGUAGES C ASM)
add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/arena.c"
    # NOTE: Unused for now.
    # "src/buddy.c"
    # "src/pool.c"
)

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
