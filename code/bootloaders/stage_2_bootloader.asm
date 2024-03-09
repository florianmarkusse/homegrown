format binary as 'bin'

include 'x64.inc'
use16

define BYTES_PER_SECTOR 512
define TOTAL_SECTORS 64
define BOOTLOADER_START 0x7E00

org BOOTLOADER_START
    mov     sp, BOOTLOADER_START   ; SP is loaded with the start, we can use all that memory below code as stack.
    mov     [drive_number], dl

    jmp     elevate_bios

include 'util.inc'

drive_number rb 1

;
; Define the Flat Mode Configuration Global Descriptor Table (GDT)
; The flat mode table allows us to read and write code anywhere, without restriction
;
gdt_32_start:

; Define the null sector for the 32 bit gdt
; Null sector is required for memory integrity check
gdt_32_null:
    dd 0x00000000           ; All values in null entry are 0
    dd 0x00000000           ; All values in null entry are 0

; Define the code sector for the 32 bit gdt
gdt_32_code:
    ; Base:     0x00000
    ; Limit:    0xFFFFF
    ; 1st Flags:        0b1001
    ;   Present:        1
    ;   Privelege:      00
    ;   Descriptor:     1
    ; Type Flags:       0b1010
    ;   Code:           1
    ;   Conforming:     0
    ;   Readable:       1
    ;   Accessed:       0
    ; 2nd Flags:        0b1100
    ;   Granularity:    1
    ;   32-bit Default: 1
    ;   64-bit Segment: 0
    ;   AVL:            0

    dw 0xFFFF           ; Limit (bits 0-15)
    dw 0x0000           ; Base  (bits 0-15)
    db 0x00             ; Base  (bits 16-23)
    db 10011010b       ; 1st Flags, Type flags
    db 11001111b       ; 2nd Flags, Limit (bits 16-19)
    db 0x00             ; Base  (bits 24-31)

; Define the data sector for the 32 bit gdt
gdt_32_data:
    ; Base:     0x00000
    ; Limit:    0xFFFFF
    ; 1st Flags:        0b1001
    ;   Present:        1
    ;   Privelege:      00
    ;   Descriptor:     1
    ; Type Flags:       0b0010
    ;   Code:           0
    ;   Expand Down:    0
    ;   Writeable:      1
    ;   Accessed:       0
    ; 2nd Flags:        0b1100
    ;   Granularity:    1
    ;   32-bit Default: 1
    ;   64-bit Segment: 0
    ;   AVL:            0

    dw 0xFFFF           ; Limit (bits 0-15)
    dw 0x0000           ; Base  (bits 0-15)
    db 0x00             ; Base  (bits 16-23)
    db 10010010b       ; 1st Flags, Type flags
    db 11001111b       ; 2nd Flags, Limit (bits 16-19)
    db 0x00             ; Base  (bits 24-31)

gdt_32_end:

; Define the gdt descriptor
; This data structure gives cpu length and start address of gdt
; We will feed this structure to the CPU in order to set the protected mode GDT
gdt_32_descriptor:
    dw gdt_32_end - gdt_32_start - 1        ; Size of GDT, one byte less than true size
    dd gdt_32_start                         ; Start of the 32 bit gdt

; Define helpers to find pointers to Code and Data segments
code_seg    equ gdt_32_code - gdt_32_start
data_seg    equ gdt_32_data - gdt_32_start

; This function will raise our CPU to the 32-bit protected mode
elevate_bios:
    ; 32-bit protected mode requires the GDT, so we tell the CPU where
    ; it is with the 'lgdt' command
    lgdt    [gdt_32_descriptor]

    ; Enable 32-bit mode by setting bit 0 of the original control
    ; register. We cannot set this directly, so we need to copy the
    ; contents into eax (32-bit version of ax) and back again
    mov     eax, cr0
    or      eax, 0x00000001
    mov     cr0, eax

    ; Now we need to clear the pipeline of all 16-bit instructions,
    ; which we do with a far jump. The address doesn't actually need to
    ; be far away, but the type of jump needs to be specified as 'far'
    jmp     code_seg:init_pm

    use32
    init_pm:

    ; Congratulations! You're now in 32-bit mode!
    ; There's just a bit more setup we need to do before we're ready
    ; to actually execute instructions

    ; We need to tell all segment registers to point to our flat-mode data
    ; segment. If you're curious about what all of these do, you might want
    ; to look on the OSDev Wiki. We will not be using them enough to matter.
    mov     ax, data_seg
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    ; Since the stack pointers got messed up in the elevation process, and we
    ; want a fresh stack, we need to reset them now.
    mov     ebp, 0x90000
    mov     esp, ebp

    ; Go to the second sector with 32-bit code
    jmp     begin_protected

begin_protected:
    call    clear_protected

    mov     esi, protected_alert
    mov     ecx, protected_alert.length
    call    print_protected

    jmp     _end_32
    
; Clear the VGA memory. (AKA write blank spaces to every character slot)
; This function takes no arguments
clear_protected:
    cld
    mov     ecx, vga_chars
    mov     edi, vga_start
    mov     ah, style_wb
    clear_protected_loop:
        mov     al, ' '
        stosw
        loop    clear_protected_loop
        ret

_end_32:
    jmp $

; Simple 32-bit protected print routine
; Message address stored in esi
; Message length in ecx
print_protected:
    cld
    mov     edi, vga_start
    mov     ah, style_wb
    print_protected_loop:
        lodsb
        stosw

        loop print_protected_loop
        ret

; Define necessary constants
define vga_start    0x000B8000
define vga_chars    80 * 25
define vga_end      0x000B8000 + 80 * 25 * 2
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
