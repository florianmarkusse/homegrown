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
   ;  biosWrite 'a', 'b', 'c', ' '
   ;  biosWrite 'a', myVar, 'b', ' ', 'f', myVar
    xor ax, ax
    ; mov ax, 10000
    mov ax, 1001
    call uint16_to_print

;    int 0x12
;    call uint16_to_print

    jmp $

print:
    mov ah, 0xE            
    int 0x10
    ret

; assumes the uint16 is in ax
uint16_to_print:
    push si
    push di
    push dx

    mov dl, 10
    mov cx, ax ; cx contains the uint16
    mov ax, 1 ; al contains 1 to start with (10^0)
    mov bl, 0   ; bl will contain the powers of 10 
    .find_power:
        biosWriteChars 'l'
        inc bl
        mul dl
        jc .found_power
        biosWriteChars 'x'
        cmp cx, ax     ; uint16 - 10^bl
        jg .find_power  ; If dx > 10^bl, continue looping

    .found_power:
        add bl, '0' 
        biosWriteChars ' ', 'b', ':', bl
        jmp .unwind

    

    .loop:
        xor dx, dx
        div si
        test dx, dx
        jz .unwind
        add dx, '0'
        biosWriteChars dl
        jmp .loop
        
    .unwind:
    pop dx
    pop di
    pop si
    ret
    

define ASCI_START 32  ; ' '
define ASCI_END   126 ; '~'
macro biosWrite charOrStrings*&
    iterate <charOrString>, charOrStrings
        if ASCI_START <= charOrString & charOrString <= ASCI_END
            biosWriteChars charOrString
        else
            biosWriteString charOrString
        end if
    end iterate
end macro

macro biosWriteString string
    push ax
    push si
    cld
    mov si, string
    repeat string.length
        lodsb   ; mov al, [si] 
                ; inc si        is the same as this
        call print
    end repeat
    pop si
    pop ax
end macro

macro biosWriteChars char&
    push ax
    iterate <chr>, char
        mov al, chr
        call print
    end iterate
    pop ax
end macro

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

