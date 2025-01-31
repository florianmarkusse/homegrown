project(efi-acpi LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/rsdp.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${REPO_ROOT}/projects/efi/code/acpi/include
)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)
