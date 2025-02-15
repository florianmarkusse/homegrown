cmake_minimum_required(VERSION 3.22)
project(platform-abstraction LANGUAGES C)
set(AGGREGATED_INTERFACE "${PROJECT_NAME}-i")
add_library(${AGGREGATED_INTERFACE} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
endif()

### NOTE: This does not seem to work recursively, see platform-abstraction-efi
### which needs to link to x86-gdt too for some reason. It should propogate
### x86-efi's link targets imo
function(include_and_link_object_library project)
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:${project}>)
    ### NOTE: Does it require below ? Why do I need the interface libraries of an implementation?
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
add_subdirectory(efi)

## NOTE: temporary until we improve the abstraction structure
add_subdirectory(physical)
add_subdirectory(virtual)
add_subdirectory(interrupts)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
