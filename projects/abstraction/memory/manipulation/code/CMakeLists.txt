create_abstraction_targets(abstraction-memory-manipulation)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    add_subproject("freestanding")
    include_and_link_object_library(freestanding-memory-manipulation)
elseif("${ENVIRONMENT}" STREQUAL "efi")
    add_subproject("freestanding")
    include_and_link_object_library(freestanding-memory-manipulation)
elseif("${ENVIRONMENT}" STREQUAL "posix")
    # Posix should link with a stdlib that contains the right implementations
endif()
