project(abstraction-interrupts LANGUAGES C ASM)
add_library(${PROJECT_NAME} INTERFACE)
set(INTERFACE_PROJECT "${PROJECT_NAME}-i")
add_library(${INTERFACE_PROJECT} INTERFACE)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    include("${REPO_PROJECTS}/abstractions.cmake")
endif()

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${INTERFACE_PROJECT}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ARCHITECTURE}" STREQUAL "x86")
    add_subproject("x86")
    if("${ENVIRONMENT}" STREQUAL "freestanding")
        include_and_link_object_library(x86-idt-real)
    elseif("${ENVIRONMENT}" STREQUAL "efi")
        include_and_link_object_library(x86-idt-real)
    elseif("${ENVIRONMENT}" STREQUAL "posix")
        include_and_link_object_library(x86-idt-mock)
    else()
        message(FATAL_ERROR "Could not match ENVIRONMENT variable")
    endif()
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
