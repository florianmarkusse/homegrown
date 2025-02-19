create_abstraction_targets(abstraction-efi)

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
