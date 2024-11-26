cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(kernel LANGUAGES C ASM)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    add_link_options("--ld-path=${CMAKE_LINKER}")

    add_subproject("shared")
    add_subproject("platform-abstraction")
    add_subproject("x86")

    if(NOT "${UNIT_TEST_BUILD}")
        set(CMAKE_C_FLAGS
            # TODO: SSE is still broken!!!
            "${CMAKE_C_FLAGS} -fno-pic -fno-pie -mcmodel=kernel -m64 -fno-stack-protector -mno-sse -mno-sse2"
        )
        set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
    else()
    endif()
endif()

# This is a C-only flag. Don't add this to the ASM flags
set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} --embed-dir=${REPO_PROJECTS}/kernel/resources"
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/print-configuration.cmake")
endif()

add_subdirectory(cpu)
add_subdirectory(memory)
add_subdirectory(status)
add_subdirectory(log)
add_subdirectory(peripheral)

if("${UNIT_TEST_BUILD}" OR NOT (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR))
    if("${UNIT_TEST_BUILD}")
        add_subproject("posix")
    endif()
    return()
endif()

################################################################################
#       End of common setup, below is only for production kernel build         #
################################################################################

add_executable(${PROJECT_NAME} "src/main.c")
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        LINK_FLAGS
            "-T ${CMAKE_CURRENT_SOURCE_DIR}/main.ld -no-pie -Wl,-n,-m,elf_x86_64,--build-id=none"
        EXCLUDE_FROM_ALL TRUE
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/shared/code/assert/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE platform-abstraction-log)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory)

target_link_libraries(${PROJECT_NAME} PRIVATE kernel-memory-management)
target_link_libraries(${PROJECT_NAME} PRIVATE kernel-peripheral-screen)
target_link_libraries(${PROJECT_NAME} PRIVATE kernel-cpu)
target_link_libraries(${PROJECT_NAME} PRIVATE kernel-status-memory)

set(BINARIZER "${PROJECT_NAME}.bin")
set(PROJECT_NAME_BIN "${PROJECT_NAME}.bin")
set(KERNEL_MAGIC_FOLDER
    "${REPO_PROJECTS}/interoperation/code/include/interoperation/generated"
)
set(KERNEL_MAGIC_OUTPUT "${KERNEL_MAGIC_FOLDER}/kernel-magic.h")
add_custom_target(
    ${BINARIZER}
    ALL
    COMMAND
        objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}
    COMMAND
        mkdir -p ${KERNEL_MAGIC_FOLDER} && python3
        "${REPO_PROJECTS}/interoperation/code/create_magic.py"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}" ${KERNEL_MAGIC_OUTPUT}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    COMMAND_EXPAND_LISTS
    BYPRODUCTS ${KERNEL_MAGIC_OUTPUT}
    VERBATIM
    COMMENT "Transforming ${PROJECT_NAME} into ${PROJECT_NAME_BIN}"
)
add_dependencies(${BINARIZER} ${PROJECT_NAME})
