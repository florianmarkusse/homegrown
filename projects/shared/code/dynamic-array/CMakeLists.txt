project(shared-dynamic-array LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/dynamic-array.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-macros)
