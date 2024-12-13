project(platform-abstraction-log LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    include_and_link_object_library(kernel-log)
elseif("${ENVIRONMENT}" STREQUAL "posix")
    include_and_link_object_library(posix-log)
else()
    message(FATAL_ERROR "Could not match ENVIRONMENT variable")
endif()

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
