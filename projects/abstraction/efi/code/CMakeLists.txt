project(abstraction-efi LANGUAGES C ASM)
add_library(${PROJECT_NAME} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    include("${REPO_PROJECTS}/abstractions.cmake")
endif()

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subproject("shared")
include_interface_library(shared-types)

if("${ARCHITECTURE}" STREQUAL "x86")
    add_subproject("x86")
    add_subproject("x86-efi")
    include_and_link_object_library(x86-efi)
    include_and_link_object_library(x86-gdt)
    include_and_link_object_library(x86-configuration)
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
