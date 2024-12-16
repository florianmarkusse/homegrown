project(platform-abstraction-memory-manipulation LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    include_and_link_object_library(kernel-memory-manipulation)
elseif("${ENVIRONMENT}" STREQUAL "posix")
else()
    # Posix should link with a stdlib that contains the right implementations
endif()

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
