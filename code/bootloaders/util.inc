; Print string pointed to by DS:SI using
; BIOS TTY output via int 10h/AH=0eh
bios_print_string:
    push    ax
    push    si
    push    bx
    xor     bx, bx
    mov     ah, 0xE       ; int 10h 'print char' function

_repeat:
    lodsb
    int     0x10
    loop    _repeat
done:
    mov ah, 0x03      
    int     0x10
    mov ah, 0x02      
    inc dh             ; Move to the next row
    xor dl, dl         ; Move to the first column
    int     0x10
    pop bx
    pop si
    pop ax
    ret

struc db? values&
      . db values
      .length = $ - .
end struc

