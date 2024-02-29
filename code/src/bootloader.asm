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
    biosWrite 'a', 'b', 'c', ' '
    biosWrite 'a', myVar, 'b', ' ', 'f', myVar

    jmp $

print:
    mov ah, 0xE            
    int 0x10
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

