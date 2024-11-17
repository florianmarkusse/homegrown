project(status-memory LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/memory.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries( PRIVATE cpu)
target_link_libraries( PRIVATE memory-management)
target_link_libraries( PRIVATE log)

target_link_libraries( PRIVATE shared-text)
