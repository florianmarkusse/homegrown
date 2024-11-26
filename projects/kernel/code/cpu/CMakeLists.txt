project(kernel-cpu LANGUAGES C ASM)

set(SOURCE_FILES)

if("${UNIT_TEST_BUILD}")
    list(APPEND SOURCE_FILES "src/posix/idt.c" "src/posix/x86.c")
else()
    list(
        APPEND
        SOURCE_FILES
        "src/kernel/idt.c"
        "src/kernel/idts.S"
        "src/kernel/x86.c"
    )
endif()

add_library(${PROJECT_NAME} OBJECT ${SOURCE_FILES})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
