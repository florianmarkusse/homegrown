#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

x86_64-testos-elf-gdb -ex "target remote localhost:1234" -ex "file projects/kernel/code/build/kernel-Debug"
