project(quickie LANGUAGES C)

list(APPEND TEST_SOURCE_FILES "src/main.c")

set(TESTS "${TARGET_PREFIX}-${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_executable(${TESTS} ${TEST_SOURCE_FILES})

target_include_directories(
    ${TESTS}
    PRIVATE ${REPO_PROJECTS}/kernel/code/${PROJECT_NAME}/include
)

target_include_directories(
    ${TESTS}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)

target_link_libraries(
    ${TESTS}
    PRIVATE ${TARGET_PREFIX}-util-${CMAKE_BUILD_TYPE}
)
target_link_libraries(
    ${TESTS}
    PRIVATE ${TARGET_PREFIX}-test-framework-${CMAKE_BUILD_TYPE}
)

message(STATUS "the kernel librarys is ${KERNEL_LIBRARY}")
message(STATUS "the impoqted location  is ${IMPORTED_LIB_LOCATION}")

target_link_libraries(${TESTS} PRIVATE ${KERNEL_LIBRARY})

target_include_directories(${TESTS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
