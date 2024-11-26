project(x86-cpu-real LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/idt.c" "src/idts.S" "src/cpu.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
