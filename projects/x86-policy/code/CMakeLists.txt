cmake_minimum_required(VERSION 3.22)
project(x86-policy LANGUAGES C ASM)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    add_subproject("shared")
    add_subproject("efi-to-kernel")
    add_platform_abstraction_and_correct_implementations()
endif()

add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/init.c"
    "src/policy.c"
    # "src/status.c"
    "src/virtual.c"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-physical)
target_link_libraries(${PROJECT_NAME} PRIVATE x86-virtual)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
