cmake_minimum_required(VERSION 3.22)
project(platform-abstraction LANGUAGES C)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    add_link_options("--ld-path=${CMAKE_LINKER}")
endif()

# TODO: Change calls with this
function(include_and_link_object_library project)
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:${project}>)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:${project},INCLUDE_DIRECTORIES>
    )
endfunction()

function(include_interface_library project)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:${project},INTERFACE_INCLUDE_DIRECTORIES>
    )
endfunction()

add_subdirectory(log)
add_subdirectory(cpu)
add_subdirectory(memory)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
