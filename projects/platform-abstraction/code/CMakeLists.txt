cmake_minimum_required(VERSION 3.22)
project(platform-abstraction LANGUAGES C)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    add_link_options("--ld-path=${CMAKE_LINKER}")
endif()

# TODO: Change calls with this
function(include_and_link_project project)
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:${project}>)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:${project},INCLUDE_DIRECTORIES>
    )
endfunction()

add_subdirectory(log)
add_subdirectory(cpu)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
