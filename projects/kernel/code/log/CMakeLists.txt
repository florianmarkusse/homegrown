project(log LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/log.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC peripheral-screen)
target_link_libraries(${PROJECT_NAME} PUBLIC memory-manipulation)
target_link_libraries(${PROJECT_NAME} PUBLIC shared-text)
