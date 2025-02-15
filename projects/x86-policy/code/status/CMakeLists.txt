project(x86-policy-status LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/status.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-policy)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-virtual)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-physical-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
