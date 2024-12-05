cmake_minimum_required(VERSION 3.22)
project(uefi-image-creator LANGUAGES C ASM)
include(${REPO_PROJECTS}/setup.cmake)
include(${REPO_PROJECTS}/print-configuration.cmake)
add_link_options(--ld-path=${CMAKE_LINKER})

add_subproject("shared")
add_platform_abstraction_and_correct_implementations()

add_executable(${PROJECT_NAME} "src/main.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-log)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-assert)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-dynamic-array)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
