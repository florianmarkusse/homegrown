project(shared-text LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/converter.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
