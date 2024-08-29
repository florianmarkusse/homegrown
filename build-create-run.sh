#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

YELLOW='\033[33m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

BUILD_MODES=("Release" "Debug" "Profiling" "Fuzzing")
BUILD_MODE="${BUILD_MODES[0]}"
C_COMPILER=$(whereis clang-19 | awk '{ print $2 }')
RUN_QEMU=true
USE_AVX=true
USE_SSE=true

function display_usage() {
    echo -e "${RED}${BOLD}Usage: $0 [${YELLOW}OPTIONS${RED}]${NO_COLOR}"
    echo -e "${BOLD}Options:${NO_COLOR}"
    echo -e "  -m, --build-mode <TYPE>    Set the build mode (${YELLOW}${BUILD_MODES[*]}${NO_COLOR}). Default is ${YELLOW}${BUILD_MODES[0]}${NO_COLOR}."
    echo -e "  -c, --c-compiler           Set the c-compiler. Default is ${YELLOW}${C_COMPILER}${NO_COLOR}."
    echo -e "  -n, --no-run           Do not run QEMU after building. Default is ${RUN_QEMU}."
    echo -e "  --no-avx                   Disable AVX. (This is applied to the operating system build only)"
    echo -e "  --no-sse                   Disable SSE. (This is applied to the operating system build only)"
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
    -c | --c-compiler)
        C_COMPILER="$2"
        shift 2
        ;;
    -n | --no-run)
        RUN_QEMU=false
        shift
        ;;
    --no-avx)
        USE_AVX=false
        shift
        ;;
    --no-sse)
        USE_SSE=false
        shift
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
    -c "$C_COMPILER"
)

[ "$USE_AVX" == "false" ] && BUILD_OPTIONS+=(--no-avx)
[ "$USE_SSE" == "false" ] && BUILD_OPTIONS+=(--no-sse)

projects/build.sh "${BUILD_OPTIONS[@]}"

find projects/uefi/code/build -executable -type f -name "uefi-${BUILD_MODE}" -exec cp {} BOOTX64.EFI \;
find projects/kernel/code/build -executable -type f -name "kernel-${BUILD_MODE}.bin" -exec cp {} kernel.bin \;
find projects/uefi-image-creator/code/build -type f -name "uefi-image-creator-${BUILD_MODE}" -exec {} --data-size 32 -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin \;

[ "$RUN_QEMU" = false ] && exit 0

RUN_QEMU_OPTIONS=(
    -o test.hdd
    -u bios.bin
)

[ "$BUILD_MODE" = "Debug" ] && RUN_QEMU_OPTIONS+=(-d) && RUN_QEMU_OPTIONS+=(-v)

./run-qemu.sh "${RUN_QEMU_OPTIONS[@]}"
