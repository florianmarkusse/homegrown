project(efi-firmware LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} INTERFACE shared-i)
target_link_libraries(${PROJECT_NAME} INTERFACE efi-i)
