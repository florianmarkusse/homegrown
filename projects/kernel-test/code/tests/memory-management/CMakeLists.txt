project(memory-management-tests LANGUAGES C)

list(APPEND TEST_SOURCE_FILES "src/main.c")

set(TESTS "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_executable(${TESTS} ${TEST_SOURCE_FILES})

include("../add-kernel-folder.cmake")
addkernelfolder("quickie")

target_include_directories(
    ${TESTS}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)

target_link_libraries(${TESTS} PRIVATE util-${CMAKE_BUILD_TYPE})
target_link_libraries(${TESTS} PRIVATE test-framework-${CMAKE_BUILD_TYPE})

target_include_directories(${TESTS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
