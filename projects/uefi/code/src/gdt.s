.global load_gdt
load_gdt:
    lgdt [rdi]      ; load GDT, rdi (1st argument) contains the gdt_ptr
;    mov ax, 0x10    ; kernel data segment is 0x10
;    mov ds, ax      ; load kernel data segment in data segment registers
;    mov es, ax
;    mov fs, ax
;    mov gs, ax
;    mov ss, ax
;    popq rdi        ; pop the return address
;    mov rax, 0x08   ; kernel code segment is 0x08
;    pushq rax       ; push the kernel code segment
;    pushq rdi       ; push the return address again
;    lretq           ; do a far return, like a normal return but
;                    ; pop an extra argument of the stack
;                    ; and load it into CS
