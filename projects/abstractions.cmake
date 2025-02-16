### NOTE: This does not seem to work recursively, see platform-abstraction-efi
### which needs to link to x86-gdt too for some reason. It should propogate
### x86-efi's link targets imo
function(include_and_link_object_library project)
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:${project}>)
    ### NOTE: Does it require below ? Why do I need the interface libraries of an implementation?
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:${project},INCLUDE_DIRECTORIES>
    )
endfunction()

function(include_interface_library project)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:${project},INTERFACE_INCLUDE_DIRECTORIES>
    )
endfunction()
