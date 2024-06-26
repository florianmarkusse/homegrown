cmake_minimum_required(VERSION 3.18)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(kernel LANGUAGES C ASM)

include("../../setup.cmake")
initial_setup()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-pic -fno-pie -mcmodel=kernel -nostdinc -nostdlib -m64 -ffreestanding -fno-stack-protector -mno-sse -mno-sse2")

add_library(${EXECUTABLE_NAME} OBJECT
        "src/main.c" 
        "src/acpi/signature.c"
        "src/idt.c"
        "src/idts.S"
        )

    target_include_directories(${EXECUTABLE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${EXECUTABLE_NAME} PUBLIC "${REPO_ROOT}/projects/interoperation/code/include")

set(LINKING_LIBRARIES
    util)
set(LINKING_LIBRARIES_LOCATIONS)
foreach(LINK_LIBRARY ${LINKING_LIBRARIES})
    add_subdirectory(${LINK_LIBRARY})
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${LINK_LIBRARY}-${CMAKE_BUILD_TYPE})
    list(APPEND LINKING_LIBRARIES_LOCATIONS ${CMAKE_CURRENT_BINARY_DIR}/${LINK_LIBRARY}/lib${LINK_LIBRARY}-${CMAKE_BUILD_TYPE}.a)
endforeach()

set(RESOURCES_NAME "resources")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCES_NAME})
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${RESOURCES_NAME}")
file(GLOB_RECURSE RESOURCE_FILES ${RESOURCE_DIR}/*)
set(RESOURCE_OBJECTS)
foreach(RESOURCE_FILE ${RESOURCE_FILES})
    string(REPLACE "${RESOURCE_DIR}/" "" RESOURCE_OBJECT ${RESOURCE_FILE})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" LOCAL_RESOURCE_FILE ${RESOURCE_FILE})
    set(RESOURCE_OBJECT "${RESOURCE_OBJECT}.o")

    set(RESOURCE_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/${RESOURCES_NAME}/${RESOURCE_OBJECT}")
    list(APPEND RESOURCE_OBJECTS ${RESOURCE_OBJECT})

    add_custom_command(
        OUTPUT ${RESOURCE_OBJECT}
        COMMAND ${CMAKE_LINKER} -r -b binary -o ${RESOURCE_OBJECT} ${LOCAL_RESOURCE_FILE}
        DEPENDS ${LOCAL_RESOURCE_FILE}
        VERBATIM
        COMMAND_EXPAND_LISTS
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Creating binary from ${LOCAL_RESOURCE_FILE} into ${RESOURCE_OBJECT}"
    )
endforeach()
add_custom_target(embedded-resources ALL DEPENDS ${RESOURCE_OBJECTS})

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}
        COMMAND ${CMAKE_LINKER} -n -m elf_x86_64 -T ${CMAKE_CURRENT_SOURCE_DIR}/main.ld 
            -o ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME} 
            $<TARGET_OBJECTS:${EXECUTABLE_NAME}> 
            ${RESOURCE_OBJECTS} 
            ${LINKING_LIBRARIES_LOCATIONS}
        COMMAND objdump -M x86_64 -D ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME} > ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.asm
        COMMAND objdump -x ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME} > ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.objdump
        COMMAND_EXPAND_LISTS
        VERBATIM
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.asm ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.objdump
        DEPENDS 
            $<TARGET_OBJECTS:${EXECUTABLE_NAME}>
            ${CMAKE_CURRENT_SOURCE_DIR}/main.ld 
            ${LINKING_LIBRARIES_LOCATIONS}
            ${RESOURCE_OBJECTS} 
        COMMENT "Linking ${EXECUTABLE_NAME}"
)

set(EXECUTABLE_NAME_BIN "${EXECUTABLE_NAME}.bin")
set(KERNEL_MAGIC_OUTPUT "${REPO_ROOT}/projects/interoperation/code/include/generated/kernel-magic.h")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME_BIN}
        COMMAND objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME_BIN}
        COMMAND python "${REPO_ROOT}/projects/interoperation/code/src/create_magic.py" "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME_BIN}" ${KERNEL_MAGIC_OUTPUT}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}
        COMMAND_EXPAND_LISTS
        BYPRODUCTS ${KERNEL_MAGIC_OUTPUT}
        VERBATIM
        COMMENT "Copying binary data from ${EXECUTABLE_NAME} into ${EXECUTABLE_NAME_BIN}"
)

# TODO: check if this gets rebuilt when util or any other library changes.
add_custom_target("${EXECUTABLE_NAME}-final" ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME_BIN})
