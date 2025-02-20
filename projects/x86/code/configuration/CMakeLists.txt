project(x86-configuration LANGUAGES C ASM)
add_library(${PROJECT_NAME} OBJECT "src/features.c" "src/cpu2.c")

add_includes_for_sublibrary()

target_link_libraries(${PROJECT_NAME} PRIVATE shared-i)
