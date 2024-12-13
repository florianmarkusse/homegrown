project(kernel-memory-management LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/policy.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE interoperation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
