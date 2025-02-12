project(efi-memory LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/physical.c")

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-firmware)
target_link_libraries(${PROJECT_NAME} PRIVATE efi-globals)
