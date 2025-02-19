macro(create_abstraction_targets target)
    project(${target} LANGUAGES C ASM)
    add_library(${target} INTERFACE)
    update_added_projects(${target})
    set(INTERFACE_OF_TARGET "${target}-i")
    add_library(${INTERFACE_OF_TARGET} INTERFACE)

    target_include_directories(
        ${target}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_include_directories(
        ${INTERFACE_OF_TARGET}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endmacro()

macro(add_includes_for_sublibrary)
    get_target_property(target_type ${PROJECT_NAME} TYPE)
    if(${target_type} STREQUAL "INTERFACE_LIBRARY")
        target_include_directories(
            ${PROJECT_NAME}
            INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
    else()
        target_include_directories(
            ${PROJECT_NAME}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
    endif()
    target_include_directories(
        ${AGGREGATED_INTERFACE}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endmacro()
