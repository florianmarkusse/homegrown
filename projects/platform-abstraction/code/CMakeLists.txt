cmake_minimum_required(VERSION 3.22)
project(platform-abstraction LANGUAGES C)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("../../setup.cmake")
    include("../../print-configuration.cmake")
    add_subdirectory(
        "${REPO_PROJECTS}/shared/code"
        "${CMAKE_CURRENT_BINARY_DIR}/shared"
    )
endif()

add_subdirectory(log)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE shared-log)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
