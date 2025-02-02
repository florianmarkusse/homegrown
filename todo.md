- Fix code so that it compiles :)
- remove **asm** in favor of asm and **volatile** for volatile
- can I get rid of EFICALL ? I am compiling with efi stuff anyway which automatically does the right ABI afaik
- Rethink memory allocation for kernel structures in uefi and whether or not to add them to free physical memory in kernel --- definitely some bugs now.
- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments

CPU features to implement/turn on in x86

- fpu
- sse
- avx
- gpe
