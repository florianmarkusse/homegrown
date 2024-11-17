project(status-idt LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/status.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries( PRIVATE peripheral-screen)
target_link_libraries( PRIVATE log)
target_link_libraries( PRIVATE shared-text)
target_link_libraries( PRIVATE cpu)
