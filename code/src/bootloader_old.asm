include 'x64.inc'

format binary as 'iso'

define BYTES_PER_SECTOR 512
define BOOTLOADER_START 0x7C00
define BOOTLOADER_MAGIC 0xAA55

org BOOTLOADER_START
    cli

    mov ax, 0                   ; Set up the data segment to 0
    mov ds, ax                  ; Initialize data segment
    mov ss, ax                  ; Initialize stack segment (SS) to 0
    mov sp, BOOTLOADER_START    ; Initialize the Stack Pointer to the bootloader location

    mov ax, 0x2401
    int 0x15
    jc .a_20_bad

    mov al, 0x20
    mov ah, 0xE
    int 0x10

.wait_loop:    
    in     al, 64h    			 ; Get status register value
    and    al, 10b    			 ; Test bit 1 of status register
    jz     .wait_loop   			 ; If status register bit not set, no data is in buffer
    in     al, 60h    			 ; Its set--Get the byte from the buffer (Port 0x60), and store it
    mov ah, 0xE
    int 0x10

    mov al, 0x20
    mov ah, 0xE
    int 0x10

    push 0xB00B
    call hex_print

.a_20_bad:
    push 0x0A20
    call hex_print
    jmp .general_bad

.general_bad:
    push 0x0BAD
    call hex_print
    jmp $

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

