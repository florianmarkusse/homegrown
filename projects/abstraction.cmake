function(abstraction_add_sources dependent)
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:${dependent}>)
endfunction()

function(include_interface_library target dependent)
    target_include_directories(
        ${target}
        INTERFACE $<TARGET_PROPERTY:${dependent},INTERFACE_INCLUDE_DIRECTORIES>
    )
endfunction()

function(abstraction_include_interface_library dependent)
    include_interface_library(${PROJECT_NAME} ${dependent})
    include_interface_library(${INTERFACE_OF_TARGET} ${dependent})
endfunction()

function(include_definition target keyValue)
    target_compile_definitions(${target} INTERFACE ${keyValue})
endfunction()

function(abstraction_include_definition keyValue)
    include_definition(${PROJECT_NAME} ${keyValue})
    include_definition(${INTERFACE_OF_TARGET} ${keyValue})
endfunction()
