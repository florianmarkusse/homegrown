project(platform-abstraction-memory-management-status LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ARCHITECTURE}" STREQUAL "x86")
    target_sources(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_OBJECTS:x86-memory-status>
    )
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:x86-memory-status,INCLUDE_DIRECTORIES>
    )
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
