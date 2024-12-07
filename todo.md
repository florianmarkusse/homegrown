- add verbose flag to cmake , and make possible to just add any -D comma-separated
- Turn on CPPcheck?
- Turn on clang static analyzer
- Turn on IKOS
- Turn on UBsan
- Turn on ASan
- Fix header guards, make command for this like --errors-to-file for iwyu?

- Why the fuck has uefi grown so large? Interface includes???
  -rwxrwxr-x 1 florian florian 44032 dec 7 20:51 uefi\*
  -rwxrwxr-x 1 florian florian 17096 dec 7 20:51 uefi\* // WTF am I doing wrong that the one above is so large???

- Start rewrite of image-builder
