.section .text
.code64

// The C isr handler
.extern fault_handler

// The current data segment we are using for the kernel
#define KERNEL_DATA_SEG (1 << 4)

// Some macros for pushing and popping all the registers of x86-64
// Although only used once, it is nice to do it this way to ensure that the stack between the two are the same

.macro pushaq
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
.endm

.macro popaq
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax
.endm

//////////////////
// asm_lidt(*void)
// Loads an idt from a pointer to an IDTR
.globl asm_lidt
asm_lidt:
    lidt (%rdi)
    ret

//////
// isr_stub()
// called by the separate isr handlers. Gives a
// uniform method of handling an isr
.globl isr_stub
isr_stub:
    // Push all the 64-bit registers
    pushaq
    // Push all the data segments (Stack alignment is kept because all segments add up to 64-bits here)
    movw %ds, %ax
    pushq %rax
    movw %es, %ax
    pushq %rax
    movw %fs, %ax
    pushq %rax
    movw %gs, %ax
    pushq %rax
    // Load the kernel data segments
    movw $KERNEL_DATA_SEG, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    // And we get into it
    movq %rsp, %rdi
    cld
    call fault_handler
    // Pop all the data segments
    popq %rax
    movw %ax, %gs
    popq %rax
    movw %ax, %fs
    popq %rax
    movw %ax, %es
    popq %rax
    movw %ax, %ds
    // And pop all the registers
    popaq
    addq $16, %rsp // Pops the error number off the stack
    //addq $16, %rsp // Pops the interrupt number off the stack
    iretq

// We'll start by making all of our ISRs. From 0 to 255

// Create an ISR wrapper for each table
// Arg 1: ISR number
.macro isr_wrapper_no_error isr_num
    .globl isr\isr_num
isr\isr_num:
    // Turn off interrupts
    cli
    // Push an error number of 0
    pushq $0
    // Push the interrupt number
    pushq $\isr_num
    jmp isr_stub // And go to the isr_stub
.endm

.macro isr_wrapper_with_error isr_num
    .globl isr\isr_num
isr\isr_num:
    // Turn off interrupts
    cli
    // Push the interrupt number
    pushq $\isr_num
    jmp isr_stub // And go to the isr_stub
.endm

isr_wrapper_no_error 0
isr_wrapper_no_error 1
isr_wrapper_no_error 2
isr_wrapper_no_error 3
isr_wrapper_no_error 4
isr_wrapper_no_error 5
isr_wrapper_no_error 6
isr_wrapper_no_error 7
isr_wrapper_with_error 8
isr_wrapper_no_error 9
isr_wrapper_with_error 10
isr_wrapper_with_error 11
isr_wrapper_with_error 12
isr_wrapper_with_error 13
isr_wrapper_with_error 14
isr_wrapper_no_error 15
isr_wrapper_no_error 16
isr_wrapper_with_error 17
isr_wrapper_no_error 18
isr_wrapper_no_error 19
isr_wrapper_no_error 20
isr_wrapper_with_error 21
isr_wrapper_no_error 22
isr_wrapper_no_error 23
isr_wrapper_no_error 24
isr_wrapper_no_error 25
isr_wrapper_no_error 26
isr_wrapper_no_error 27
isr_wrapper_no_error 28
isr_wrapper_no_error 29
isr_wrapper_no_error 30
isr_wrapper_no_error 31
isr_wrapper_no_error 32
isr_wrapper_no_error 33
isr_wrapper_no_error 34
isr_wrapper_no_error 35
isr_wrapper_no_error 36
isr_wrapper_no_error 37
isr_wrapper_no_error 38
isr_wrapper_no_error 39
isr_wrapper_no_error 40
isr_wrapper_no_error 41
isr_wrapper_no_error 42
isr_wrapper_no_error 43
isr_wrapper_no_error 44
isr_wrapper_no_error 45
isr_wrapper_no_error 46
isr_wrapper_no_error 47
isr_wrapper_no_error 48
isr_wrapper_no_error 49
isr_wrapper_no_error 50
isr_wrapper_no_error 51
isr_wrapper_no_error 52
isr_wrapper_no_error 53
isr_wrapper_no_error 54
isr_wrapper_no_error 55
isr_wrapper_no_error 56
isr_wrapper_no_error 57
isr_wrapper_no_error 58
isr_wrapper_no_error 59
isr_wrapper_no_error 60
isr_wrapper_no_error 61
isr_wrapper_no_error 62
isr_wrapper_no_error 63
isr_wrapper_no_error 64
isr_wrapper_no_error 65
isr_wrapper_no_error 66
isr_wrapper_no_error 67
isr_wrapper_no_error 68
isr_wrapper_no_error 69
isr_wrapper_no_error 70
isr_wrapper_no_error 71
isr_wrapper_no_error 72
isr_wrapper_no_error 73
isr_wrapper_no_error 74
isr_wrapper_no_error 75
isr_wrapper_no_error 76
isr_wrapper_no_error 77
isr_wrapper_no_error 78
isr_wrapper_no_error 79
isr_wrapper_no_error 80
isr_wrapper_no_error 81
isr_wrapper_no_error 82
isr_wrapper_no_error 83
isr_wrapper_no_error 84
isr_wrapper_no_error 85
isr_wrapper_no_error 86
isr_wrapper_no_error 87
isr_wrapper_no_error 88
isr_wrapper_no_error 89
isr_wrapper_no_error 90
isr_wrapper_no_error 91
isr_wrapper_no_error 92
isr_wrapper_no_error 93
isr_wrapper_no_error 94
isr_wrapper_no_error 95
isr_wrapper_no_error 96
isr_wrapper_no_error 97
isr_wrapper_no_error 98
isr_wrapper_no_error 99
isr_wrapper_no_error 100
isr_wrapper_no_error 101
isr_wrapper_no_error 102
isr_wrapper_no_error 103
isr_wrapper_no_error 104
isr_wrapper_no_error 105
isr_wrapper_no_error 106
isr_wrapper_no_error 107
isr_wrapper_no_error 108
isr_wrapper_no_error 109
isr_wrapper_no_error 110
isr_wrapper_no_error 111
isr_wrapper_no_error 112
isr_wrapper_no_error 113
isr_wrapper_no_error 114
isr_wrapper_no_error 115
isr_wrapper_no_error 116
isr_wrapper_no_error 117
isr_wrapper_no_error 118
isr_wrapper_no_error 119
isr_wrapper_no_error 120
isr_wrapper_no_error 121
isr_wrapper_no_error 122
isr_wrapper_no_error 123
isr_wrapper_no_error 124
isr_wrapper_no_error 125
isr_wrapper_no_error 126
isr_wrapper_no_error 127
isr_wrapper_no_error 128
isr_wrapper_no_error 129
isr_wrapper_no_error 130
isr_wrapper_no_error 131
isr_wrapper_no_error 132
isr_wrapper_no_error 133
isr_wrapper_no_error 134
isr_wrapper_no_error 135
isr_wrapper_no_error 136
isr_wrapper_no_error 137
isr_wrapper_no_error 138
isr_wrapper_no_error 139
isr_wrapper_no_error 140
isr_wrapper_no_error 141
isr_wrapper_no_error 142
isr_wrapper_no_error 143
isr_wrapper_no_error 144
isr_wrapper_no_error 145
isr_wrapper_no_error 146
isr_wrapper_no_error 147
isr_wrapper_no_error 148
isr_wrapper_no_error 149
isr_wrapper_no_error 150
isr_wrapper_no_error 151
isr_wrapper_no_error 152
isr_wrapper_no_error 153
isr_wrapper_no_error 154
isr_wrapper_no_error 155
isr_wrapper_no_error 156
isr_wrapper_no_error 157
isr_wrapper_no_error 158
isr_wrapper_no_error 159
isr_wrapper_no_error 160
isr_wrapper_no_error 161
isr_wrapper_no_error 162
isr_wrapper_no_error 163
isr_wrapper_no_error 164
isr_wrapper_no_error 165
isr_wrapper_no_error 166
isr_wrapper_no_error 167
isr_wrapper_no_error 168
isr_wrapper_no_error 169
isr_wrapper_no_error 170
isr_wrapper_no_error 171
isr_wrapper_no_error 172
isr_wrapper_no_error 173
isr_wrapper_no_error 174
isr_wrapper_no_error 175
isr_wrapper_no_error 176
isr_wrapper_no_error 177
isr_wrapper_no_error 178
isr_wrapper_no_error 179
isr_wrapper_no_error 180
isr_wrapper_no_error 181
isr_wrapper_no_error 182
isr_wrapper_no_error 183
isr_wrapper_no_error 184
isr_wrapper_no_error 185
isr_wrapper_no_error 186
isr_wrapper_no_error 187
isr_wrapper_no_error 188
isr_wrapper_no_error 189
isr_wrapper_no_error 190
isr_wrapper_no_error 191
isr_wrapper_no_error 192
isr_wrapper_no_error 193
isr_wrapper_no_error 194
isr_wrapper_no_error 195
isr_wrapper_no_error 196
isr_wrapper_no_error 197
isr_wrapper_no_error 198
isr_wrapper_no_error 199
isr_wrapper_no_error 200
isr_wrapper_no_error 201
isr_wrapper_no_error 202
isr_wrapper_no_error 203
isr_wrapper_no_error 204
isr_wrapper_no_error 205
isr_wrapper_no_error 206
isr_wrapper_no_error 207
isr_wrapper_no_error 208
isr_wrapper_no_error 209
isr_wrapper_no_error 210
isr_wrapper_no_error 211
isr_wrapper_no_error 212
isr_wrapper_no_error 213
isr_wrapper_no_error 214
isr_wrapper_no_error 215
isr_wrapper_no_error 216
isr_wrapper_no_error 217
isr_wrapper_no_error 218
isr_wrapper_no_error 219
isr_wrapper_no_error 220
isr_wrapper_no_error 221
isr_wrapper_no_error 222
isr_wrapper_no_error 223
isr_wrapper_no_error 224
isr_wrapper_no_error 225
isr_wrapper_no_error 226
isr_wrapper_no_error 227
isr_wrapper_no_error 228
isr_wrapper_no_error 229
isr_wrapper_no_error 230
isr_wrapper_no_error 231
isr_wrapper_no_error 232
isr_wrapper_no_error 233
isr_wrapper_no_error 234
isr_wrapper_no_error 235
isr_wrapper_no_error 236
isr_wrapper_no_error 237
isr_wrapper_no_error 238
isr_wrapper_no_error 239
isr_wrapper_no_error 240
isr_wrapper_no_error 241
isr_wrapper_no_error 242
isr_wrapper_no_error 243
isr_wrapper_no_error 244
isr_wrapper_no_error 245
isr_wrapper_no_error 246
isr_wrapper_no_error 247
isr_wrapper_no_error 248
isr_wrapper_no_error 249
isr_wrapper_no_error 250
isr_wrapper_no_error 251
isr_wrapper_no_error 252
isr_wrapper_no_error 253
isr_wrapper_no_error 254
isr_wrapper_no_error 255


