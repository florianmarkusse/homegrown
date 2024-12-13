- Use functions in platform-abstraction project
- Remove duplicate function definitions and use platform-abstraction definition instead
- Fix remaining warnings
- Do enum stuff
- interfaces in libraries!
- Fix iwyu

- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments

### Size of LTO'ed before logging changes

-rw-rw-r-- 1 florian florian 779945 dec 9 22:23 kernel.asm
