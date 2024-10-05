project(peripheral-screen LANGUAGES C ASM)

set(SOURCE_FILES)

if("${UNIT_TEST_BUILD}")
    list(APPEND SOURCE_FILES "src/posix/screen.c")
else()
    list(APPEND SOURCE_FILES "src/kernel/screen.c")
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_ROOT}/projects/kernel/resources"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC memory-management)
target_link_libraries(${PROJECT_NAME} PUBLIC memory-manipulation)
target_link_libraries(${PROJECT_NAME} PUBLIC cpu)
target_link_libraries(${PROJECT_NAME} PUBLIC util)
