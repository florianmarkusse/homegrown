project(platform-abstraction-interrupts LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ARCHITECTURE}" STREQUAL "x86")
    if("${ENVIRONMENT}" STREQUAL "freestanding")
        include_and_link_object_library(x86-idt-real)
    elseif("${ENVIRONMENT}" STREQUAL "efi")
        include_and_link_object_library(x86-idt-real)
    elseif("${ENVIRONMENT}" STREQUAL "posix")
        include_and_link_object_library(x86-idt-mock)
    else()
        message(FATAL_ERROR "Could not match ENVIRONMENT variable")
    endif()
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
