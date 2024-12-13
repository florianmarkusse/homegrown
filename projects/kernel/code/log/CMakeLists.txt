project(kernel-log LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/log.c")

add_subdirectory(init)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)

target_link_libraries(${PROJECT_NAME} PRIVATE kernel-peripheral-screen)
target_link_libraries(${PROJECT_NAME} PRIVATE kernel-log-init)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-log)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-sizes)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-assert)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-macros)
