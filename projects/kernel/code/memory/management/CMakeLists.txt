# TODO: maybe split this up in virtual/physical/whatever at
# a later stage.
project(memory-management LANGUAGES C ASM)

if("${UNIT_TEST_BUILD}")
    add_subdirectory(tests)
endif()

add_library(
    ${PROJECT_NAME}
    STATIC
    "src/physical.c"
    "src/virtual.c"
    "src/policy.c"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC cpu)
target_link_libraries(${PROJECT_NAME} PUBLIC memory-manipulation)

target_link_libraries(${PROJECT_NAME} PUBLIC shared-maths)

target_link_libraries(${PROJECT_NAME} PUBLIC x86-memory)
