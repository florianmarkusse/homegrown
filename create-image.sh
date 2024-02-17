#!/bin/bash
set -eo pipefail

cp code/build/*.elf bootboot-in/initdir/mykernel
cd bootboot-in
MKBOOT=$(find ../dependencies/bootboot -type f -name "mkbootimg")
"${MKBOOT}" config.json ../bootboot-out/mykernel.iso
