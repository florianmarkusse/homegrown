# Interoperation

This project aims to provide implementations that:

- make communicating between different platforms, uefi -> kernel, possible. And,
- provide solutions that pick the right platform-dependent code, so
  - a logging solution that uses the POSIX or Kernel logger based on the specified platform
  - Hence, this is not the place for platform-INdependent code
