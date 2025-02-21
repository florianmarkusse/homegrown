create_abstraction_targets(abstraction-memory-manipulation)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    add_subproject("freestanding")
    abstraction_add_sources(freestanding-memory-manipulation)
elseif("${ENVIRONMENT}" STREQUAL "efi")
    add_subproject("freestanding")
    abstraction_add_sources(freestanding-memory-manipulation)
elseif("${ENVIRONMENT}" STREQUAL "posix")
    # Posix should link with a stdlib that contains the right implementations
endif()
