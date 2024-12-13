project(kernel-peripheral-screen LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/screen.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/kernel/resources"
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction)
target_link_libraries(${PROJECT_NAME} PRIVATE shared)
target_link_libraries(${PROJECT_NAME} PRIVATE interoperation)
target_link_libraries(${PROJECT_NAME} PRIVATE x86)
