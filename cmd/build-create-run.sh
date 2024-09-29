#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"
cd ../

YELLOW='\033[33m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

BUILD_MODES=("Release" "Debug")
BUILD_MODE="${BUILD_MODES[0]}"

function display_usage() {
    echo -e "${RED}${BOLD}Usage: $0 [${YELLOW}OPTIONS${RED}]${NO_COLOR}"
    echo -e "${BOLD}Options:${NO_COLOR}"
    echo -e "  -m, --build-mode <TYPE>    Set the build mode (${YELLOW}${BUILD_MODES[*]}${NO_COLOR}). Default is ${YELLOW}${BUILD_MODES[0]}${NO_COLOR}."
    echo -e "  -h, --help                 Display this help message."
    exit 1
}

function is_valid_build_mode() {
    local mode="$1"
    for valid_mode in "${BUILD_MODES[@]}"; do
        if [[ "$mode" == "$valid_mode" ]]; then
            return 0
        fi
    done
    return 1
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
    -m | --build-mode)
        if ! is_valid_build_mode "$2"; then
            echo -e "${RED}${BOLD}Invalid ${YELLOW}BUILD_MODE${RED}. Valid options: ${NO_COLOR}${YELLOW}${BUILD_MODES[*]}${NO_COLOR}."
            exit 1
        fi
        BUILD_MODE="$2"
        shift 2
        ;;
    -h | --help)
        display_usage
        ;;
    *)
        display_usage
        ;;
    esac
done

BUILD_OPTIONS=(
    -m "${BUILD_MODE}"
)

cmd/compile.elf "${BUILD_OPTIONS[@]}"

find projects/uefi/code/build -executable -type f -name "uefi-${BUILD_MODE}" -exec cp {} BOOTX64.EFI \;
find projects/kernel/code/build -executable -type f -name "kernel-${BUILD_MODE}.bin" -exec cp {} kernel.bin \;
find projects/uefi-image-creator/code/build -type f -name "uefi-image-creator-${BUILD_MODE}" -exec {} --data-size 32 -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin \;

RUN_QEMU_OPTIONS=(
    -o test.hdd
    -u bios.bin
)

[ "$BUILD_MODE" = "Debug" ] && RUN_QEMU_OPTIONS+=(-d) && RUN_QEMU_OPTIONS+=(-v)

cmd/run-qemu.elf "${RUN_QEMU_OPTIONS[@]}"
