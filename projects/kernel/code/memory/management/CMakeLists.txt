# TODO: maybe split this up in virtual/physical/whatever at
# a later stage.
project(kernel-memory-management LANGUAGES C ASM)

if("${BUILD_UNIT_TESTS}")
    add_subdirectory(tests)
endif()

add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/physical.c"
    "src/virtual.c"
    "src/policy.c"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-cpu)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-assert)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-macros)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory)
