project(util LANGUAGES C ASM)

set(LIBRARY "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
set(LIBRARY_TYPE "STATIC")

add_library(${LIBRARY} ${LIBRARY_TYPE}
    "src/memory/memory.c"
    "src/memory/arena.c"
    "src/memory/buddy-allocator.c"
    "src/memory/pool-allocator.c"
    "src/log.c"
)

target_include_directories(${LIBRARY} PUBLIC "${REPO_ROOT}/projects/interoperation/code/include")

target_include_directories(${LIBRARY}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
