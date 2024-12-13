project(platform-abstraction-memory-management LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

add_subdirectory(status)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ARCHITECTURE}" STREQUAL "x86")
    include_and_link_object_library(x86-memory)
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
