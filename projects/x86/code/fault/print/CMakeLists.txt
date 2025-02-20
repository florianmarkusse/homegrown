project(x86-fault-print LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/core.c")

add_includes_for_sublibrary()

if("${UNIT_TEST_BUILD}")
    add_subdirectory(test)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-log-i)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-i)
