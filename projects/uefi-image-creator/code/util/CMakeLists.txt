project(util LANGUAGES C)

add_library(
    ${PROJECT_NAME}
    STATIC
    "src/file/read.c"
    "src/file/path.c"
    "src/hash/msi/common.c"
    "src/hash/msi/string-set.c"
    "src/hash/msi/uint16-set.c"
    "src/hash/trie/string-set.c"
    "src/hash/trie/string-uint16-map.c"
    "src/hash/trie/uint16-set.c"
    "src/hash/trie/string-auto-uint16-map.c"
    "src/hash/hash-comparison-status.c"
    "src/log.c"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PUBLIC shared-memory-allocator)
target_link_libraries(${PROJECT_NAME} PUBLIC shared-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PUBLIC shared-text)
