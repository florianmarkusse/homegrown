project(platform-abstraction-memory-manipulation LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    target_sources(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_OBJECTS:kernel-memory-manipulation>
    )
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE
            $<TARGET_PROPERTY:kernel-memory-manipulation,INCLUDE_DIRECTORIES>
    )
elseif("${ENVIRONMENT}" STREQUAL "posix")
    target_sources(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_OBJECTS:posix-memory-manipulation>
    )
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE
            $<TARGET_PROPERTY:posix-memory-manipulation,INCLUDE_DIRECTORIES>
    )
else()
    message(FATAL_ERROR "Could not match ENVIRONMENT variable")
endif()
