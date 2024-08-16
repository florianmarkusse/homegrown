project(quickie LANGUAGES C)

list(APPEND TEST_SOURCE_FILES "src/main.c")

set(TESTS "${TARGET_PREFIX}-${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")

add_executable(${TESTS} ${TEST_SOURCE_FILES})

set(TEST_OBJECT "test-object")
# Add the kernel project as an external project
ExternalProject_Add(
    ${TEST_OBJECT}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/${TEST_OBJECT}
    SOURCE_DIR ${REPO_PROJECTS}/kernel/code/${PROJECT_NAME}
    CMAKE_ARGS -DUNIT_TEST=ON
    INSTALL_COMMAND
        "" # Disable install step
    LOG_CONFIGURE
        ON # Enable verbose logging for configure step
    LOG_BUILD
        ON # Enable verbose logging for build step
)

ExternalProject_Get_Property(${TEST_OBJECT} BINARY_DIR)
add_library(OBJECT_LIBRARY STATIC IMPORTED) # Adjust STATIC to SHARED if needed
set_target_properties(
    OBJECT_LIBRARY
    PROPERTIES IMPORTED_LOCATION "${BINARY_DIR}/lib${PROJECT_NAME}-.a"
)

add_dependencies(${TESTS} ${TEST_OBJECT}) # Ensure kernel builds before tests
target_link_libraries(${TESTS} PRIVATE OBJECT_LIBRARY)

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

target_include_directories(${TESTS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
