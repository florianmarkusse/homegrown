project(platform-abstraction-log LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${FREESTANDING_BUILD}")
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:kernel-log>)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:kernel-log,INCLUDE_DIRECTORIES>
    )
else()
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:posix-log>)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:posix-log,INCLUDE_DIRECTORIES>
    )
endif()
