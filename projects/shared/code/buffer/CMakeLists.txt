project(shared-buffer LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/buffer.c")

# target_include_directories(
#     ${PROJECT_NAME}
#     PUBLIC
#         $<BUILD_INTERFACE:${REPO_PROJECTS}/interoperation/code/include>
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#         $<INSTALL_INTERFACE:include>
# )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC shared-text)
target_link_libraries(${PROJECT_NAME} PUBLIC shared-memory-management)
target_link_libraries(${PROJECT_NAME} PUBLIC shared-memory-allocator)

# install(
#     TARGETS "${PROJECT_NAME}"
#     EXPORT sharedTargets
#     ARCHIVE DESTINATION lib
#     INCLUDES DESTINATION include
# )
