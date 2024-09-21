#ifndef CPU_JMP_H
#define CPU_JMP_H

typedef void *jmp_buf[5];
#define setjmp __builtin_setjmp
#define longjmp __builtin_longjmp

#endif
