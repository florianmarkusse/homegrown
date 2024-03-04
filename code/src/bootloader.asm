include 'x64.inc'

format binary as 'iso'

define BYTES_PER_SECTOR 512
define BOOTLOADER_START 0x7C00
define BOOTLOADER_MAGIC 0xAA55

org BOOTLOADER_START
    ;
    ; This jump is not strictly necessary, but some BIOS will start you at 0x07C0:0000
    ; which is in fact the linear address as 0x0:7C00, but the range of jump will be
    ; different. We will unify that with this long jump.
    ;
    jmp 0:true_start

true_start:
    cli     ; disable interrupts

    ; We zero the segment registers
    xor     ax, ax
    mov     ds, ax
    mov     es, ax
    mov     ss, ax

    mov     sp, BOOTLOADER_MAGIC   ; SP is loaded with 0x7C00, we can use all that memory below code as stack.

; Enabling A20...
;   mov ax, 0x2401
;   int 0x15
;   This also works? But not for all things of course, pepelaugh
seta20.1:
    in        al, 0x64   ; Wait for not busy
    test      al, 0x2
    jnz       seta20.1

    mov       al, 0xd1   ; 0xd1 -> port 0x64
    out       0x64, al

seta20.2:
    in      al, 0x64     ; Wait for not busy
    test    al, 0x2
    jnz     seta20.2

    mov     al, 0xdf      ; 0xdf -> port 0x60
    out     0x60, al
; =======================

; We need to store our drive number onto the stack (DL)
    push    dx   ; Let's just push the entire DX register

; Clear Screen and set video mode to 2
    mov     ah, 0
    mov     al, 2
    int     0x10

    mov     si, loading_string
    mov     ecx, loading_string.length
    call    print_string
    pop     dx  


; Prepare for the BIOS call to load the next few sectors to
; memory.
;
; First test to make sure LBA addressing mode is supported. This
; is generally not supported on floppy drives
;
    mov     ah, 0x41
    mov     bx, 0x55aa
    int     0x13
    jc      error_no_ext_load
    cmp     bx, 0xaa55
    jnz     error_no_ext_load

; If all is well, we will load the first 64x(512B) blocks to 0x7E00
    mov     si, stage_2.length
    mov     ah, 0x42
    int     0x13
    jc      error_load ; Carry is set if there is error while loading
    mov     si, success_string
    mov     ecx, success_string.length
    call    print_string
    jmp     _end

; goto stage 2
    jmp     0:0x7E00
_end:
    jmp $

error_no_ext_load:
    mov     si, error_string_no_ext
    mov     ecx, error_string_no_ext.length
    call    print_string
    jmp     _end

error_load:
    mov     si, error_string_load
    mov     ecx, error_string_load.length
    call    print_string
    jmp     _end

; Print string pointed to by DS:SI using
; BIOS TTY output via int 10h/AH=0eh

print_string:
    push    ax
    push    si
    push    bx
    xor     bx, bx
    mov     ah, 0xe       ; int 10h 'print char' function

_repeat:
    lodsb
    int     0x10
    loop    _repeat
    mov al, 0x0A
    int     0x10
    mov ah, 0x02      
    inc dh             ; Move to the next row
    xor dl, dl         ; Move to the first column
    int     0x10
done:
    pop bx
    pop si
    pop ax
    ret

struc db? values&
      . db values
      .length = $ - .
end struc

loading_string db 'Loading Stage 2'
error_string_no_ext db 'no EXT load'
error_string_load db 'Failed to load sectors'
success_string db 'Stage 2 loaded! Now starting stage 2'

struc DISK_ADDRESS_BLOCK 
    .length db              0x10    ; length of this block
    .reserved db            0x0     ; reserved
    .number_of_blocks dw    64      ; number of blocks = 32k/512b = 64K
    .target_address dd      0x07E00000  ; Target memory address
    .starting_block dq      1       ; Starting Disk block 1, since we just need to skip the boot sector.
end struc
stage_2 DISK_ADDRESS_BLOCK

; hex_print:
;     push bp
;     mov bp, sp
; 
;     mov bx, 4
;     ._loop:
;         cmp bx, 0
;         jz ._my_end
;         dec bx
;         mov ax, 4
;         mov cx, 3
;         sub cx, bx
;         mul cx
; 
;         mov cx, ax
;         mov ax, word [bp + 4]
; 
;         shl ax, cl
;         shr ax, 12
; 
;         cmp al, 10
;         jl ._num
;         mov ah, 55 ; '0' - 10
;         jmp ._char
;         ._num:
;             mov ah, '0'
;         ._char:
;             add al, ah
;             mov ah, 0xE
;             int 0x10
;             jmp ._loop
;     ._my_end:
;         pop bp
;         ret 2


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

