project(kernel-peripheral-screen LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/screen.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/kernel/resources"
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-cpu)

target_link_libraries(${PROJECT_NAME} PRIVATE kernel-memory-management)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-assert)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-macros)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory)
