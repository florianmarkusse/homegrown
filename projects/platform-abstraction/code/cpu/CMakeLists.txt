project(platform-abstraction-cpu LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ARCHITECTURE}" STREQUAL "x86")
    if("${ENVIRONMENT}" STREQUAL "freestanding")
        target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:x86-cpu-real>)
        target_include_directories(
            ${PROJECT_NAME}
            INTERFACE $<TARGET_PROPERTY:x86-cpu-real,INCLUDE_DIRECTORIES>
        )
    elseif("${ENVIRONMENT}" STREQUAL "posix")
        target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:x86-cpu-mock>)
        target_include_directories(
            ${PROJECT_NAME}
            INTERFACE $<TARGET_PROPERTY:x86-cpu-mock,INCLUDE_DIRECTORIES>
        )
    else()
        message(FATAL_ERROR "Could not match ENVIRONMENT variable")
    endif()
else()
    message(FATAL_ERROR "Could not match ARCHITECTURE variable")
endif()
