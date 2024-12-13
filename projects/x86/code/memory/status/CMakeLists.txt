project(x86-memory-status LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/status.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-log)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)

target_link_libraries(${PROJECT_NAME} PRIVATE interoperation)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory-definitions)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-assert)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-macros)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-management)
