format binary as 'bin'

include 'x64.inc'
use16

define BYTES_PER_SECTOR 512
define TOTAL_SECTORS 64
define BOOTLOADER_START 0x7E00

org BOOTLOADER_START
    mov     sp, BOOTLOADER_START   ; SP is loaded with the start, we can use all that memory below code as stack.
    mov     [drive_number], dl

    mov     si, stage_2_string
    mov     ecx, stage_2_string.length
    call    bios_print_string
    
    jmp     elevate_bios

include 'util.inc'

stage_2_string db 'Reached stage 2'
drive_number rb 1

include 'gdt.inc'

use32

begin_protected:
call clear_protected

; Clear the VGA memory. (AKA write blank spaces to every character slot)
; This function takes no arguments
clear_protected:
    ; The pusha command stores the values of all
    ; registers so we don't have to worry about them
    pusha

    ; Set up constants
    mov ebx, vga_extent
    mov ecx, vga_start
    mov edx, 0

    ; Do main loop
    clear_protected_loop:
        ; While edx < ebx
        cmp edx, ebx
        jge clear_protected_done

        ; Free edx to use later
        push edx

        ; Move character to al, style to ah
        mov al, ' '
        mov ah, style_wb

        ; Print character to VGA memory
        add edx, ecx
        mov word[edx], ax

        ; Restore edx
        pop edx

        ; Increment counter
        add edx,2

        ; GOTO beginning of loop
        jmp clear_protected_loop

clear_protected_done:
    ; Restore all registers and return
    popa

    mov esi, protected_alert
    call print_protected

_end_32:
    jmp $

; Simple 32-bit protected print routine
; Message address stored in esi
print_protected:
    ; The pusha command stores the values of all
    ; registers so we don't have to worry about them
    pusha
    mov edx, vga_start

    ; Do main loop
    print_protected_loop:
        ; If char == \0, string is done
        cmp byte[esi], 0
        je  print_protected_done

        ; Move character to al, style to ah
        mov al, byte[esi]
        mov ah, style_wb

        ; Print character to vga memory location
        mov word[edx], ax

        ; Increment counter registers
        add esi, 1
        add edx, 2

        ; Redo loop
        jmp print_protected_loop

print_protected_done:
    ; Popa does the opposite of pusha, and restores all of
    ; the registers
    popa
    ret

; Define necessary constants
define vga_start    0x000B8000
define vga_extent   80 * 25 * 2             ; VGA Memory is 80 chars wide by 25 chars tall (one char is 2 bytes)
define style_wb     0x0F

; Define messages
protected_alert db 'Now in 32-bit protected mode'

; stage_2 is hardcoded to be 32Kib.
if $ - $$ <= BYTES_PER_SECTOR * TOTAL_SECTORS
    rb (BYTES_PER_SECTOR * TOTAL_SECTORS - ($ - $$))
else 
    err  ; code out of sector bounds!
end if
