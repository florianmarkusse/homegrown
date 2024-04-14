#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

BUILD_MODES=("Release" "Debug" "Profiling" "Fuzzing")
BUILD_MODE="${BUILD_MODES[0]}"
C_COMPILER=$(whereis clang-19 | awk '{ print $2 }')

function display_usage() {
	echo -e "${RED}${BOLD}Usage: $0 [${YELLOW}OPTIONS${RED}]${NO_COLOR}"
	echo -e "${BOLD}Options:${NO_COLOR}"
	echo -e "  -m, --build-mode <TYPE>    Set the build mode (${YELLOW}${BUILD_MODES[*]}${NO_COLOR}). Default is ${YELLOW}${BUILD_MODES[0]}${NO_COLOR}."
	echo -e "  -c, --c-compiler           Set the c-compiler. Default is ${YELLOW}${C_COMPILER}${NO_COLOR}."
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
	-h | --help)
		display_usage
		;;
	*)
		display_usage
		;;
	esac
done

projects/build.sh -m "${BUILD_MODE}" -c "$C_COMPILER"
cp "projects/uefi/code/build/uefi-${BUILD_MODE}" BOOTX64.EFI
cp "projects/kernel/code/build/kernel-${BUILD_MODE}.bin" kernel.bin
"projects/uefi-image-creator/code/build/uefi-image-creator-${BUILD_MODE}" -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin

RUN_QEMU_OPTIONS=(
	-o test.hdd
	-u bios.bin
)

[ "$BUILD_MODE" = "Debug" ] && RUN_QEMU_OPTIONS+=(-d)

./run-qemu.sh "${RUN_QEMU_OPTIONS[@]}"
