- merge efi-setup.cmake
- Environments - add EFI_ENVIRONMENT , and maybe add definition WITH_STD and WITHOUT_STD to do other kinds of includes in platform-abstraction
- Remove if def cplusplus. If I ever add it, just use a tool
- Rethink memory allocation for kernel structures in uefi and whether or not to add them to free physical memory in kernel --- definitely some bugs now.
- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments

CPU features to implement/turn on in x86

- fpu
- sse
- avx
- gpe

X86_ARCHITECTURE
FREESTANDING_ENVIRONMENT EFI_ENVIRONMENT POSIX_ENVIRONMENT
PROJECT_BUILD UNIT_TEST_BUILD
