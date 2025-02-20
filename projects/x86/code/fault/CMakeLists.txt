project(x86-fault LANGUAGES C ASM)
add_library(${PROJECT_NAME} INTERFACE)

add_includes_for_sublibrary()

add_subdirectory(print)
