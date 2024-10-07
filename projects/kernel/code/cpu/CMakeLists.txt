project(cpu LANGUAGES C ASM)

set(SOURCE_FILES)

if("${UNIT_TEST_BUILD}")
    list(APPEND SOURCE_FILES "src/posix/idt.c")
else()
    list(
        APPEND
        SOURCE_FILES
        "src/kernel/idt.c"
        "src/kernel/idts.S"
        "src/kernel/x86.c"
    )
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

if("${UNIT_TEST_BUILD}")
    target_link_libraries(${PROJECT_NAME} PRIVATE util)
endif()

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
