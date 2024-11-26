project(x86-cpu-mock LANGUAGES C ASM)

add_library(${PROJECT_NAME} OBJECT "src/idt.c" "src/cpu.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/interoperation/code/include"
)
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE $<TARGET_PROPERTY:x86-cpu-real,INTERFACE_INCLUDE_DIRECTORIES>
)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-types)
