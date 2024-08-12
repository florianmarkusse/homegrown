# Kernel Test

This project houses all the tests for the kernel.

It is a separate project since different dependencies are used such as `libc`
or the POSIX-bindings to have a faster way to test instead of having to turn on
QEMU all the time.
