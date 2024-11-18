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

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-log)

target_link_libraries(${PROJECT_NAME} PRIVATE cpu)
target_link_libraries(${PROJECT_NAME} PRIVATE memory-management)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory)
