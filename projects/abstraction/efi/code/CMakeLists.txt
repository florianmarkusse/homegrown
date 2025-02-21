create_abstraction_targets(abstraction-efi)

add_project("shared")

if("${ARCHITECTURE}" STREQUAL "x86")
    add_project("x86")
    add_project("x86/efi")
    abstraction_add_sources(x86-efi)
    abstraction_add_sources(x86-gdt)
    abstraction_add_sources(x86-configuration)
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
