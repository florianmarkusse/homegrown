project(log LANGUAGES C ASM)

add_library(${PROJECT_NAME} STATIC "src/log.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC util)
target_link_libraries(${PROJECT_NAME} PUBLIC peripheral-screen)
target_link_libraries(${PROJECT_NAME} PUBLIC memory-manipulation)
target_link_libraries(${PROJECT_NAME} PUBLIC text)
