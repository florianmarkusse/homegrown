#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

YELLOW='\033[33m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

IMAGE_LOCATION="bootboot-out/mykernel.iso"
USE_GDB=false

function display_usage() {
	echo -e "Usage: $0 [${YELLOW}OPTIONS${NO_COLOR}]"
	echo -e "${BOLD}Options:${NO_COLOR}"
	echo -e "  -i, --image-location       Set the .iso location. Default is ${YELLOW}${IMAGE_LOCATION}${NO_COLOR}."
	echo -e "  -g, --use-gdb              Use gdb to debug."
	echo -e "  -h, --help                 Display this help message."
	exit 1
}

function display_single_flag_configuration() {
	local flag="$1"
	local option="$2"

	if [ "$flag" = true ]; then
		echo -e "${BOLD}${YELLOW}${option}${NO_COLOR}: ${YELLOW}Yes${NO_COLOR}"
	else
		echo -e "${BOLD}${YELLOW}${option}${NO_COLOR}: ${YELLOW}No${NO_COLOR}"
	fi
}

function display_configuration() {
	echo -e "${BOLD}${YELLOW}Configuration...${NO_COLOR}"

	echo -e "${BOLD}${YELLOW}.iso location${NO_COLOR}: ${YELLOW}${IMAGE_LOCATION}${NO_COLOR}"

	display_single_flag_configuration $USE_GDB "gdb"

	echo ""
}

while [[ "$#" -gt 0 ]]; do
	case $1 in
	-i | --image-location)
		IMAGE_LOCATION="$2"
		shift 2
		;;
	-g | --use-gdb)
		USE_GDB=true
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

display_configuration

QEMU_OPTIONS=(
	-accel "tcg,thread=single"
	-cpu core2duo
	-m 128
	-no-reboot
	-drive "format=raw,media=cdrom,file=$IMAGE_LOCATION"
	-serial stdio
	-smp 1
	-usb
	-vga std
)

if [ "$USE_GDB" = true ]; then
	QEMU_OPTIONS+=(
		-s -S
	)
fi

echo -e "${BOLD}qemu-system-x86_64 ${QEMU_OPTIONS[*]}${NO_COLOR}"
qemu-system-x86_64 "${QEMU_OPTIONS[@]}"
