# TODO: maybe split this up in virtual/physical/whatever at
# a later stage.
project(memory-management LANGUAGES C ASM)

set(LIBRARY "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
set(LIBRARY_TYPE "STATIC")

if("${UNIT_TEST_BUILD}")
    add_subdirectory(tests)
endif()

add_library(
    ${LIBRARY}
    ${LIBRARY_TYPE}
    "src/allocator/arena.c"
    "src/allocator/buddy.c"
    "src/allocator/pool.c"
    "src/physical.c"
    "src/virtual.c"
)

target_include_directories(
    ${LIBRARY}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)
target_include_directories(${LIBRARY} PUBLIC "${CMAKE_SOURCE_DIR}/util/include")
target_include_directories(
    ${LIBRARY}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${LIBRARY} PUBLIC cpu-${CMAKE_BUILD_TYPE})
target_link_libraries(${LIBRARY} PUBLIC text-${CMAKE_BUILD_TYPE})
target_link_libraries(${LIBRARY} PUBLIC log-${CMAKE_BUILD_TYPE})
