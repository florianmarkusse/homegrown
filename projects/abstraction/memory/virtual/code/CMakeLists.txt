create_abstraction_targets(abstraction-memory-virtual)

if("${ARCHITECTURE}" STREQUAL "x86")
    add_subproject("x86")
    add_subproject("x86-virtual")
    include_and_link_object_library(x86-virtual)
    include_interface_library(x86-memory)
    target_compile_definitions(
        ${PROJECT_NAME}
        INTERFACE
            ABSTRACTION_VIRTUAL_MEMORY_CONVERTER="x86/memory/definitions.h"
    )
else()
    message(FATAL_ERROR "Could not find matching lib for ${PROJECT_NAME}")
endif()
