#!/bin/bash
set -eo pipefail
set -x

cd "$(dirname "${BASH_SOURCE[0]}")"

code/build.sh
cp code/uefi/build/uefi-Release BOOTX64.EFI
cp code/kernel/build/kernel-Release.bin kernel.bin
code/uefi-image-creator/build/uefi-image-creator-Release -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin

./run-qemu.sh -o test.hdd -u bios.bin
