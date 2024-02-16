; boot.asm
hang:
    jmp hang
 
    times 512-($-$$) db 0
