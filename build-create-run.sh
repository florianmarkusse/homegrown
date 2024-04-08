#!/bin/bash
set -eo pipefail
set -x

cd "$(dirname "${BASH_SOURCE[0]}")"

projects/build.sh
cp projects/uefi/code/build/uefi-Release BOOTX64.EFI
cp projects/kernel/code/build/kernel-Release.bin kernel.bin
projects/uefi-image-creator/code/build/uefi-image-creator-Release -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin

./run-qemu.sh -o test.hdd -u bios.bin
