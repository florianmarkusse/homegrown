project(x86-memory LANGUAGES C ASM)

add_library(
    ${PROJECT_NAME}
    OBJECT
    "src/page.c"
    "src/physical.c"
    "src/virtual.c"
)

if("${BUILD_UNIT_TESTS}")
    add_subdirectory(tests)
endif()

add_subdirectory(status)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE interoperation)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-cpu)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE platform-abstraction-memory-manipulation
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-assert)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-macros)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-management)
