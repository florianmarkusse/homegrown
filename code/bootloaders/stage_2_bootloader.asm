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

    mov ax, 0xb800   ; text video memory
    mov es, ax
 
    mov bx, 0x09   ;hardware interrupt #
    shl bx, 2   ;multiply by 4
    xor ax, ax
    mov gs, ax   ;start of memory
    mov [gs:bx], word keyhandler
    mov [gs:bx+2], word ds ; segment
    sti
    
_end:
    jmp $

include 'util.inc'

stage_2_string db 'Reached stage 2'

drive_number rb 1

keyhandler:
   in al, 0x60   ; get key data
   mov bl, al   ; save it
   mov byte [port60], al
 
   in al, 0x61   ; keybrd control
   mov ah, al
   or al, 0x80   ; disable bit 7
   out 0x61, al   ; send it back
   xchg ah, al   ; get original
   out 0x61, al   ; send that back
 
   mov al, 0x20   ; End of Interrupt
   out 0x20, al   ;
 
   and bl, 0x80   ; key released
   jnz _interrupt_done   ; don't repeat
 
   mov ax, [port60]
   mov  word [reg16], ax
   call printreg16

_interrupt_done:
    iret

port60 dw 0

dochar:   call cprint         ; print one character
sprint:   lodsb      ; string char to AL
   cmp al, 0
   jne dochar   ; else, we're done
   add byte [ypos], 1   ;down one row
   mov byte [xpos], 0   ;back to left
   ret
 
cprint:   mov ah, 0x0F   ; attrib = white on black
   mov cx, ax    ; save char/attribute
   movzx ax, byte [ypos]
   mov dx, 160   ; 2 bytes (char/attrib)
   mul dx      ; for 80 columns
   movzx bx, byte [xpos]
   shl bx, 1    ; times 2 to skip attrib
 
   mov di, 0        ; start of video memory
   add di, ax      ; add y offset
   add di, bx      ; add x offset
 
   mov ax, cx        ; restore char/attribute
   stosw              ; write char/attribute
   add byte [xpos], 1  ; advance to right
 
   ret
 
;------------------------------------
 
printreg16:
   mov di, outstr16
   mov ax, [reg16]
   mov si, hexstr
   mov cx, 4   ;four places
hexloop:
   rol ax, 4   ;leftmost will
   mov bx, ax   ; become
   and bx, 0x0f   ; rightmost
   mov bl, [si + bx];index into hexstr
   mov [di], bl
   inc di
   dec cx
   jnz hexloop
 
   mov si, outstr16
   call sprint
 
   ret
 
;------------------------------------
 
xpos   db 0
ypos   db 0
hexstr   db '0123456789ABCDEF'
outstr16   db '0000', 0  ;register value string
reg16   dw    0  ; pass values to printreg16

; stage_2 is hardcoded to be 32Kib.
if $ - $$ <= BYTES_PER_SECTOR * TOTAL_SECTORS
    rb (BYTES_PER_SECTOR * TOTAL_SECTORS - ($ - $$))
else 
    err  ; code out of sector bounds!
end if
