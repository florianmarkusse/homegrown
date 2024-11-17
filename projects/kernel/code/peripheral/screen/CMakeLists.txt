project(peripheral-screen LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/screen.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/kernel/resources"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries( PRIVATE memory-management)
target_link_libraries( PRIVATE shared-memory-manipulation)
target_link_libraries( PRIVATE shared-memory-allocator)
target_link_libraries( PRIVATE cpu)
target_link_libraries( PRIVATE util)
