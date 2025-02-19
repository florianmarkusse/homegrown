project(freestanding-peripheral-screen LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/screen.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-to-kernel-i)

target_link_libraries(${PROJECT_NAME} PRIVATE freestanding-i)

target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-management-i)
target_link_libraries(${PROJECT_NAME} PRIVATE abstraction-memory-manipulation-i)
