message(STATUS "=== Configuration Settings ===")
message(STATUS "Project:                ${PROJECT_NAME}")
message(STATUS "Build type:             ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler:             ${CMAKE_C_COMPILER}")
message(STATUS "C flags:                ${CMAKE_C_FLAGS}")
get_directory_property(
    compile_definitions
    DIRECTORY ${CMAKE_SOURCE_DIR}
    COMPILE_DEFINITIONS
)
message(STATUS "Compile Definitions:    ${compile_definitions}")
message(STATUS "Assembler:              ${CMAKE_ASM_COMPILER}")
message(STATUS "Assembler Flags:        ${CMAKE_ASM_FLAGS}")
message(STATUS "Linker:                 ${CMAKE_LINKER}")
message(STATUS "repo root:              ${REPO_ROOT}")
message(STATUS "repo projects:          ${REPO_PROJECTS}")
message(STATUS "repo dependencies:      ${REPO_DEPENDENCIES}")
message(STATUS "=== End Configuration ===")
