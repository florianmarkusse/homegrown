project(platform-abstraction-cpu LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:x86-cpu-real>)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:x86-cpu-real,INCLUDE_DIRECTORIES>
    )
else()
    target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:x86-cpu-mock>)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<TARGET_PROPERTY:x86-cpu-mock,INCLUDE_DIRECTORIES>
    )
endif()
