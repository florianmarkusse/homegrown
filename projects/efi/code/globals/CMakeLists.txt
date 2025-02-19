project(efi-globals LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/globals.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)

target_link_libraries(${PROJECT_NAME} PRIVATE efi-i)
