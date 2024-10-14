project(shared-memory-allocator LANGUAGES C ASM)

add_library(
    ${PROJECT_NAME}
    STATIC
    "src/arena.c"
    # NOTE: Unused for now.
    # "src/buddy.c"
    # "src/pool.c"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC shared-memory-manipulation)
