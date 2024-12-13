project(platform-abstraction-log LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:kernel-log>)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:kernel-log,INCLUDE_DIRECTORIES>
    )
elseif("${ENVIRONMENT}" STREQUAL "posix")
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:posix-log>)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:posix-log,INCLUDE_DIRECTORIES>
    )
else()
    message(FATAL_ERROR "Could not match ENVIRONMENT variable")
endif()

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
