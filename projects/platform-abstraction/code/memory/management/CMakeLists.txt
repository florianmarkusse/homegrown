project(platform-abstraction-memory-management LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

add_subdirectory(status)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ARCHITECTURE}" STREQUAL "x86")
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:x86-memory>)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:x86-memory,INCLUDE_DIRECTORIES>
    )
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()

target_include_directories(
    ${AGGREGATED_INTERFACE}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
