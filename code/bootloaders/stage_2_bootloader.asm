include 'x64.inc'

format binary as 'bin'

define BYTES_PER_SECTOR 512
define TOTAL_SECTORS 64
define BOOTLOADER_START 0x7E00

org BOOTLOADER_START
    mov     sp, BOOTLOADER_START   ; SP is loaded with the start, we can use all that memory below code as stack.
    mov     [drive_number], dl

    mov     si, stage_2_string
    mov     ecx, stage_2_string.length
    call    bios_print_string

_end:
    jmp $

include 'util.inc'

stage_2_string db 'Reached stage 2'

drive_number rb 1

; stage_2 is hardcoded to be 32Kib.
if $ - $$ <= BYTES_PER_SECTOR * TOTAL_SECTORS
    rb (BYTES_PER_SECTOR * TOTAL_SECTORS - ($ - $$))
else 
    err  ; code out of sector bounds!
end if
