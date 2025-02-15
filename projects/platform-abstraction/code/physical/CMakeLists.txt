project(platform-abstraction-physical LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ENVIRONMENT}" STREQUAL "efi")
    include_and_link_object_library(efi-memory)
elseif("${ARCHITECTURE}" STREQUAL "x86")
    include_and_link_object_library(x86-physical)
    include_and_link_object_library(x86-physical-status)
else()
    message(FATAL_ERROR "Could not find matching lib for ${PROJECT_NAME}")
endif()

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
