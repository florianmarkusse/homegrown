project(shared-hash LANGUAGES C ASM)
add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/msi/common.c"
    "src/msi/string-set.c"
    "src/msi/u16-set.c"
    "src/trie/string-set.c"
    "src/trie/string-u16-map.c"
    "src/trie/uint16-set.c"
    "src/trie/string-auto-u16-map.c"
    "src/hash-comparison-status.c"
)

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
