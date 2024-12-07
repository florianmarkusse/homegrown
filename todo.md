// TODO: rename kernel/log.h this to kernel/log.h

- ENTRIES SHIFT thing struct clang-19
- Move basic modules from x/a/a.h to x/a.h
- A lot of projects SHOULD no longer depend on interoperation
- -fwhole-program -Wswitch-enum
- add verbose flag to cmake , and make possible to just add any -D comma-separated
- add another build mode : Production? that adds -lto
- Move cpu module into posix/kernel and combine into platform-abstraction project
- Turn on CPPcheck?
- Turn on clang static analyzer
- Turn on IKOS
- Turn on UBsan
- Turn on ASan
- Fix header guards, make command for this like --errors-to-file for iwyu?
- Start rewrite of image-builder
- Move all virtual & physical memory operations to X86 architecture. All platform-independent code should really just use policy which in turn communicates with the right architecture

-rwxrwxr-x 1 florian florian 44032 dec 7 20:51 uefi\*
-rwxrwxr-x 1 florian florian 17096 dec 7 20:51 uefi\* // WTF am I doing wrong that the one above is so large???
