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

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-assert)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-macros)
