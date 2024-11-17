project(posix-test-framework LANGUAGES C)

add_library(${PROJECT_NAME} OBJECT "src/test.c" "src/expectations.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-manipulation)

target_link_libraries(${PROJECT_NAME} PRIVATE posix-log)
