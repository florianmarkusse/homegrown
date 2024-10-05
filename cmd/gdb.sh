#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"
cd ../

x86_64-testos-elf-gdb -ex "target remote localhost:1234" -ex "dashboard -layout source stack threads assembly variables breakpoints memory history !registers !expressions" -ex "file projects/kernel/code/build/prod/clang-19/kernel"
