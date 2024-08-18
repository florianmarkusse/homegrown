project(hash LANGUAGES C ASM)

set(LIBRARY "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
set(LIBRARY_TYPE "STATIC")

add_library(
    ${LIBRARY}
    ${LIBRARY_TYPE}
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
    ${LIBRARY}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)

target_include_directories(
    ${LIBRARY}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${LIBRARY} PUBLIC util-${CMAKE_BUILD_TYPE})
