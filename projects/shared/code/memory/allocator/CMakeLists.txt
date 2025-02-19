project(shared-memory-allocator LANGUAGES C ASM)

add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/arena.c"
    # NOTE: Unused for now.
    # "src/buddy.c"
    # "src/pool.c"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
