cmake_minimum_required(VERSION 3.22)
project(x86-physical LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    add_subproject("shared")
    add_subproject("platform-abstraction")
    add_subproject("efi-to-kernel")
    add_subproject("x86")
    add_subproject("x86-virtual")
endif()

add_library(${PROJECT_NAME} OBJECT "src/x86-physical.c")

if("${UNIT_TEST_BUILD}")
    add_subdirectory(tests)
endif()

add_subdirectory(status)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
