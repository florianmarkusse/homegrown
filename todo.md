- Turn on CPPcheck?
- Turn on clang static analyzer
- Turn on IKOS
- Turn on UBsan
- Turn on ASan
- Fix header guards, make command for this like --errors-to-file for iwyu?

```
clang-tidy $(find kernel/code/ -type d \( -path kernel/code/build \) -prune -o -type f -name "*.[ch]" -print) -p ~/Desktop/homegrown/projects/kernel/code
```
