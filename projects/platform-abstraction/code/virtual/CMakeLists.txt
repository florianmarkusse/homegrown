project(platform-abstraction-virtual LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ARCHITECTURE}" STREQUAL "x86")
    include_and_link_object_library(x86-virtual)
    include_interface_library(x86-memory)
    target_compile_definitions(
        ${PROJECT_NAME}
        INTERFACE
            PLATFORM_ABSTRACTION_VIRTUAL_MEMORY_CONVERTER="x86/memory/definitions.h"
    )
else()
    message(FATAL_ERROR "Could not find matching lib for ${PROJECT_NAME}")
endif()

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
