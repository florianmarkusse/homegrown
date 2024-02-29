include 'x64.inc'

format binary as 'iso'

define BYTES_PER_SECTOR 512
define BOOTLOADER_START 0x7C00
define BOOTLOADER_MAGIC 0xAA55

org BOOTLOADER_START
    mov ax, 0                   ; Set up the data segment to 0
    mov ds, ax                  ; Initialize data segment
    mov ss, ax                  ; Initialize stack segment (SS) to 0
    mov sp, BOOTLOADER_START    ; Initialize the Stack Pointer to the bootloader location
    xor ax, ax

    push 0x1337
    call hex_print
    push 0xB00B
    call hex_print

    jmp $

print:
    mov ah, 0xE            
    int 0x10
    ret

hex_print:
    push bp
    mov bp, sp

    mov bx, 4
    ._loop:
        cmp bx, 0
        jz ._end
        dec bx
        mov ax, 4
        mov cx, 3
        sub cx, bx
        mul cx

        mov cx, ax
        mov ax, word [bp + 4]

        shl ax, cl
        shr ax, 12

        cmp al, 10
        jl ._num
        mov ah, 55 ; '0' - 10
        jmp ._char
        ._num:
            mov ah, '0'
        ._char:
            add al, ah
            mov ah, 0xE
            int 0x10
            jmp ._loop
    ._end:
        pop bp
        ret 2


struc db? values&
      . db values
      .length = $ - .
end struc

myVar db 'hello'


; BOOT FOOTER
; Not all media sectors have the same size. Two signatures are required for such media: 
; by offset 510 in the sector and at the end of the sector
if $$ + 509 + $ > 0
    rb $$ + 510 - $
else if $$ + 510 + $ < 0
    err  ; code out of sector bounds!
end if

dw BOOTLOADER_MAGIC

if BYTES_PER_SECTOR > 512
    rb $$ + BYTES_PER_SECTOR - 2 - $
    dw BOOTLOADER_MAGIC
end if    

