project(test-framework LANGUAGES C)

add_library(${PROJECT_NAME} STATIC "src/test.c" "src/expectations.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PUBLIC util)
target_link_libraries(${PROJECT_NAME} PUBLIC log)
