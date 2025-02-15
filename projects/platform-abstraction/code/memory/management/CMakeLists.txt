project(platform-abstraction-memory-management LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ARCHITECTURE}" STREQUAL "x86")
    include_and_link_object_library(x86-memory)
    include_and_link_object_library(x86-physical)
    include_and_link_object_library(x86-physical-status)
    include_and_link_object_library(x86-virtual)
    include_and_link_object_library(x86-virtual-status)
    include_and_link_object_library(x86-policy)
    include_and_link_object_library(x86-policy-status)
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
