project(bootloaders LANGUAGES ASM)

# The name of the file, the seek into the img, the number of sectors to write
list(APPEND BOOTLOADERS 
    "stage_1_bootloader.asm 0 1"
    "stage_2_bootloader.asm 1 15"
    )


set(BOOTLOADER_BINARIES)
foreach(BOOTLOADER ${BOOTLOADERS})
    string(REGEX MATCH "([^ ]+) ([^ ]+) ([^ ]+)" MATCHING_RESULT ${BOOTLOADER})
    set(BOOTLOADER_FILE ${CMAKE_MATCH_1})
    set(SEEK ${CMAKE_MATCH_2})
    set(COUNT ${CMAKE_MATCH_3})

    string(REGEX REPLACE "\\..*$" "" BINARY_OUTPUT_PATH ${BOOTLOADER_FILE})
    set(BOOTLOADER_OUTPUT_PATH ${BINARY_OUTPUT_PATH}.bin)

    set(BOOTLOADER_BINARY ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_OUTPUT_PATH})
    list(APPEND BOOTLOADER_BINARIES ${BOOTLOADER_BINARY})

    add_custom_command(
        OUTPUT ${BOOTLOADER_BINARY}
        COMMAND INCLUDE=${CMAKE_ASM_INCLUDE} ${CMAKE_ASM_COMPILER} -e 100 ${BOOTLOADER_FILE} ${BOOTLOADER_BINARY}
        COMMAND dd if=${BOOTLOADER_BINARY} of=${OS_OUTPUT} bs=512 seek=${SEEK} count=${COUNT} conv=notrunc
        DEPENDS ${BOOTLOADER_FILE}
        VERBATIM
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Assembling ${BOOTLOADER_FILE} to ${BOOTLOADER_OUTPUT_PATH} and writing into ${OS_OUTPUT}"
    )
endforeach()

add_custom_target(bootloaders ALL DEPENDS ${BOOTLOADER_BINARIES})
