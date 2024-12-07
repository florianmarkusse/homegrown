project(kernel-memory-management LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/policy.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-cpu)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-management
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-management)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-assert)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-macros)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory)
