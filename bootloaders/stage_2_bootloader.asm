format binary as 'bin'

include 'x64.inc'
use16

define BYTES_PER_SECTOR 512
define TOTAL_SECTORS 63
define BOOTLOADER_START 0x7E00
define KERNEL_START 0x4090

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

; ==============================================================================
; This is where 16 bit ends and 32 bits start
; ==============================================================================

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
    mov     ebp, BOOTLOADER_START
    mov     esp, ebp

    ; Go to the second sector with 32-bit code
    jmp     begin_protected

; The 32 bit start
;
;
;
;
;
;
;
begin_protected:
    call    clear_vga_32

    call    detect_lm_protected
    
    mov     esi, protected_alert
    mov     ecx, protected_alert.length
    call    print_vga_32

    call    elevate_protected
    
; Clear the VGA memory. (AKA write blank spaces to every character slot)
; This function takes no arguments
clear_vga_32:
    cld
    mov     eax, 0x0F200F20
    mov     edi, vga_start
    mov     ecx, vga_chars_d
    rep     stosd
    ret

; Define necessary constants
define vga_start    0x000B8000
define vga_chars_b  80 * 25 * 2             ; VGA Memory is 80 chars wide by 25 chars tall (one char is 2 bytes)
define vga_chars_w  80 * 25
define vga_chars_d  80 * 25 / 2
define vga_end      0x000B8000 + 80 * 25 * 2
define style_wb     0x0F

; Simple 32-bit protected print routine
; Message address stored in esi
; Message length in ecx
print_vga_32:
    cld
    mov     edi, vga_start
    mov     ah, style_wb
    print_vga_32_loop:
        lodsb
        stosw

        loop print_vga_32_loop
        ret

; Define messages
protected_alert db 'Now in 32-bit protected mode'


; Detecting Long Mode
;
; Long mode is supported on all 64-bit CPUs, but not
; on 32-bit CPUs. However, since we've been in protected
; mode, we have no idea which type of system we're running
; on. We need to figure out if we can even use long mode so
; that we don't cause a ton of errors when trying to activate
; it.
detect_lm_protected:
    ; In order to detect long mode, we need to use the built-in command
    ; "cpuid". However, CPUID has different modes, and some CPUS don't
    ; support the additional functionalities to check for long mode, even
    ; if they support CPUID. So this makes the check for long mode way more
    ; convoluted than it needs to be. Once again, the steps are:
    ;
    ; 1. Check for CPUID
    ; 2. Check for CPUID extended functions
    ; 3. Check for long mode support
    

    ; Checking for CPUID
    ;
    ; We can check for the existence of CPUID by using a bit in the
    ; FLAGS register. This register cannot be accessed by the mov command,
    ; so we must actually push it onto the stack and pop it off to read or
    ; write to it.
    ;
    ; The bit that tells us if CPUID is supported is bit 21. IF CPUID is
    ; NOT supported, then this bit MUST take on a certain value. However,
    ; if CPUID is supported, then it'll take on whatever value we give it.
    ; We can try this by reading from FLAGS, flipping the bit, writing to
    ; FLAGS, and then reading again and comparing to the earlier read. If
    ; they're equal, then the bit is 0

    pushfd                          ; Copy FLAGS to eax via stack
    pop     eax

    mov     ecx, eax
    ; Flip the ID bit (21st bit of eax)
    xor     eax, 1 shl 21

    ; Write to FLAGS
    push    eax
    popfd

    ; Bit will be flipped if CPUID supported
    pushfd
    pop     eax

    ; Restore eflags to the older version saved in ecx
    push    ecx
    popfd

    ; If equal, then the bit got flipped back during copy,
    ; and CPUID is not supported
    cmp     eax, ecx
    je      cpuid_not_found_protected   


    ; Check for extended functions of CPUID
    ;
    ; Now that we know CPUID exists, we can use it to
    ; see whether it supports the extended functions needed
    ; to enable long mode. The CPUID function takes an argument
    ; in eax and returns the value in eax. To check for extended
    ; capabilities, we use the argument mov eax, 0x80000000. If
    ; extended capabilities exist, then the value will be set to
    ; greater than 0x80000000, otherwise it will stay the same.
    mov     eax, 0x80000000            
    cpuid                           
    cmp     eax, 0x80000001           
    jb      cpuid_not_found_protected  


    ; Actually check for long mode
    ;
    ; Now, we can use CPUID to check for long mode. If long mode is
    ; supported, then CPUID will set the 29th bit of register edx. The
    ; eax code to look for long mode is 0x80000001
    mov     eax, 0x80000001           
    cpuid                          
    test    edx, 1 shl 29             
    jz      lm_not_found_protected     
    
    ret

cpuid_not_found_protected:
    call    clear_vga_32
    mov     esi, cpuid_not_found_str
    mov     ecx, cpuid_not_found_str.length
    call    print_vga_32
    jmp     $

lm_not_found_protected:
    call    clear_vga_32
    mov     esi, lm_not_found_str
    mov     ecx, lm_not_found_str.length
    call    print_vga_32
    jmp     $

lm_not_found_str        db 'ERROR: Long mode not supported. Exiting...'
cpuid_not_found_str     db 'ERROR: CPUID unsupported, but required for long mode'

elevate_protected:

    ; The memory pages are aligned to 4kib, so the first 12 bits are alwaus 0
    mov     edx, level_4_memory
    mov     cr3, edx

    mov     edx, level_3_memory
    or      edx, 11b
    mov     [level_4_memory], edx ; Set the lowest of the zeroed/lower half to the first address of level 3 memory
    mov     [level_4_memory + 256 * 8], edx ; Set the lowest of the ones/higher half to the first address of level 3 memory

    mov     edx, level_2_memory
    or      edx, 11b
    mov     [level_3_memory], edx

    mov     edx, level_1_memory
    or      edx, 11b
    mov     [level_2_memory], edx

    mov     edi, level_1_memory
    mov     edx, 11b                ; EDX has address 0x0000 with flags 0x0003
    mov     ecx, 512                ; Do the operation 512 times

    add_page_entry_protected:
        ; a = address, x = index of page table, flags are entry flags
        mov     [edi], edx                 ; Write ebx to PT[x] = a.append(flags)
        add     edx, 0x1000                     ; Increment address of ebx (a+1)
        add     edi, 8                          ; Increment page table location (since entries are 8 bytes)
                                                ; x++
        loop    add_page_entry_protected        ; Decrement ecx and loop again

    ; Enable paging
    mov eax, cr4
    or eax, 1 shl 5               ; Set the PAE-bit, which is the 5th bit
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 shl 8
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 shl 31
    mov cr0, eax
    
    lgdt [gdt_64_descriptor]
    jmp code_seg_64:init_lm

; ==============================================================================
; This is where 32 bit ends and 64 bits start
; ==============================================================================
use64
init_lm:
    mov     ax, data_seg_64           ; Set the A-register to the data descriptor.
    mov     ds, ax                    ; Set the data segment to the A-register.
    mov     es, ax                    ; Set the extra segment to the A-register.
    mov     fs, ax                    ; Set the F-segment to the A-register.
    mov     gs, ax                    ; Set the G-segment to the A-register.
    mov     ss, ax                    ; Set the stack segment to the A-register.

begin_long_mode:
    call    clear_vga_32

    mov     rsi, long_mode_note
    mov     rcx, long_mode_note.length
    call    print_vga_32

;    mov     rdi, DWORD [rsp]
;    mov     rsi, DWORD [rsp + 4]
;    mov     rbp, 0xFFFF800000000000
;    mov     rsp, 0xFFFF800000000000 + 0x10000
;    movabs  rax, OFFSET kmain
;    call    rax


    mov     rbp, 0xFC00
    mov     rsp, 0xFC00
    call    0xFCA3

macro align boundary,value:?
        db (boundary-1)-($+boundary-1) mod boundary dup value
end macro

;
; Define the Flat Mode Configuration Global Descriptor Table (GDT)
; The flat mode table allows us to read and write code anywhere, without restriction
;
align 4
gdt_64_start:

; Define the null sector for the 64 bit gdt
; Null sector is required for memory integrity check
gdt_64_null:
    dd 0x00000000           ; All values in null entry are 0
    dd 0x00000000           ; All values in null entry are 0

; Define the code sector for the 64 bit gdt
gdt_64_code:
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
    ;   32-bit Default: 0
    ;   64-bit Segment: 1
    ;   AVL:            0

    dw 0xFFFF           ; Limit (bits 0-15)
    dw 0x0000           ; Base  (bits 0-15)
    db 0x00             ; Base  (bits 16-23)
    db 10011010b        ; 1st Flags, Type flags
    db 10101111b        ; 2nd Flags, Limit (bits 16-19)
    db 0x00             ; Base  (bits 24-31)

; Define the data sector for the 64 bit gdt
gdt_64_data:
    ; Base:     0x00000
    ; Limit:    0x00000
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
    ;   32-bit Default: 0
    ;   64-bit Segment: 1
    ;   AVL:            0

    dw 0x0000           ; Limit (bits 0-15)
    dw 0x0000           ; Base  (bits 0-15)
    db 0x00             ; Base  (bits 16-23)
    db 10010010b        ; 1st Flags, Type flags
    db 10100000b        ; 2nd Flags, Limit (bits 16-19)
    db 0x00             ; Base  (bits 24-31)

gdt_64_end:

; Define the gdt descriptor
; This data structure gives cpu length and start address of gdt
; We will feed this structure to the CPU in order to set the protected mode GDT
gdt_64_descriptor:
    dw gdt_64_end - gdt_64_start - 1        ; Size of GDT, one byte less than true size
    dd gdt_64_start                         ; Start of the 64 bit gdt

; Define helpers to find pointers to Code and Data segments
code_seg_64 equ gdt_64_code - gdt_64_start
data_seg_64 equ gdt_64_data - gdt_64_start


; Zero out the 4 levels of page tables we will initially be using.
define MEMORY_ENTRIES 512
align 4096
level_4_memory:
    dq  MEMORY_ENTRIES dup 0

align 4096
level_3_memory:
    dq  MEMORY_ENTRIES dup 0

align 4096
level_2_memory:
    dq  MEMORY_ENTRIES dup 0

align 4096
level_1_memory:
    dq  MEMORY_ENTRIES dup 0


long_mode_note db 'Now running in fully-enabled, 64-bit long mode'

if $ - $$ <= BYTES_PER_SECTOR * TOTAL_SECTORS
    rb (BYTES_PER_SECTOR * TOTAL_SECTORS - ($ - $$))
else 
    err  ; code out of sector bounds!
end if
