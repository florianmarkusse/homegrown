cmake_minimum_required(VERSION 3.22)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(kernel LANGUAGES C ASM)
include(${REPO_PROJECTS}/setup.cmake)

if(NOT "${UNIT_TEST_BUILD}")
    set(CMAKE_C_FLAGS
        # TODO: SSE is still broken!!!
        "${CMAKE_C_FLAGS} -fno-pic -fno-pie -mcmodel=kernel -m64 -fno-stack-protector -mno-sse -mno-sse2"
    )
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
endif()

# This is a C-only flag. Don't add this to the ASM flags
set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} --embed-dir=${REPO_PROJECTS}/kernel/resources"
)

include(${REPO_PROJECTS}/print-configuration.cmake)

add_subproject("shared")
add_subproject("platform-abstraction")
add_subproject("x86")

add_subdirectory(cpu)
add_subdirectory(memory)
add_subdirectory(status)

if("${UNIT_TEST_BUILD}")
    add_subproject("posix")
    return()
endif()

################################################################################
#       End of common setup, below is only for production kernel build         #
################################################################################

add_subdirectory(log)
add_subdirectory(peripheral)

add_library(${PROJECT_NAME} OBJECT "src/main.c")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/platform-abstraction/code/log/include"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${REPO_PROJECTS}/shared/code/assert/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE shared-text)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-maths)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-manipulation)
target_link_libraries(${PROJECT_NAME} PRIVATE shared-memory-allocator)

target_link_libraries(${PROJECT_NAME} PRIVATE x86-memory)

target_link_libraries(${PROJECT_NAME} PRIVATE kernel-memory-management)
target_link_libraries(${PROJECT_NAME} PRIVATE kernel-log)
target_link_libraries(${PROJECT_NAME} PRIVATE kernel-peripheral-screen)
target_link_libraries(${PROJECT_NAME} PRIVATE kernel-cpu)
target_link_libraries(${PROJECT_NAME} PRIVATE kernel-status-memory)

get_target_property(OBJECT_LIBRARIES ${PROJECT_NAME} LINK_LIBRARIES)
set(OBJECT_TARGETS_LIBRARIES "")
foreach(LIB ${OBJECT_LIBRARIES})
    list(APPEND OBJECT_TARGETS_LIBRARIES "$<TARGET_OBJECTS:${LIB}>")
endforeach()

set(LINKER_TARGET "${PROJECT_NAME}.elf")
add_custom_target(
    ${LINKER_TARGET}
    ALL
    COMMAND
        ${CMAKE_LINKER} -n -m elf_x86_64 ${CMAKE_SHARED_LINKER_FLAGS} -T
        ${CMAKE_CURRENT_SOURCE_DIR}/main.ld -o
        ${CMAKE_CURRENT_BINARY_DIR}/${LINKER_TARGET}
        $<TARGET_OBJECTS:${PROJECT_NAME}> ${OBJECT_TARGETS_LIBRARIES}
    COMMAND
        objdump -M x86_64 -D ${CMAKE_CURRENT_BINARY_DIR}/${LINKER_TARGET} >
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.asm
    COMMAND
        objdump -x ${CMAKE_CURRENT_BINARY_DIR}/${LINKER_TARGET} >
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.objdump
    COMMAND_EXPAND_LISTS
    VERBATIM
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.asm
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.objdump
    COMMENT "Linking ${LINKER_TARGET}"
)
add_dependencies(${LINKER_TARGET} ${PROJECT_NAME})

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
        objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/${LINKER_TARGET}
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}
    COMMAND
        mkdir -p ${KERNEL_MAGIC_FOLDER} && python3
        "${REPO_PROJECTS}/interoperation/code/create_magic.py"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_BIN}" ${KERNEL_MAGIC_OUTPUT}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${LINKER_TARGET}
    COMMAND_EXPAND_LISTS
    BYPRODUCTS ${KERNEL_MAGIC_OUTPUT}
    VERBATIM
    COMMENT "Transforming ${PROJECT_NAME} into ${PROJECT_NAME_BIN}"
)
add_dependencies(${BINARIZER} ${LINKER_TARGET})
