#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

YELLOW='\033[33m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

IMAGE_LOCATION="undefined"
VERBOSE=false
DEBUG=false

function display_usage() {
	echo -e "Usage: $0 [${YELLOW}OPTIONS${NO_COLOR}]"
	echo -e "${BOLD}Options:${NO_COLOR}"
	echo -e "  -i, --image-location       Set the .iso location. Default is ${YELLOW}${IMAGE_LOCATION}${NO_COLOR}."
	echo -e "  -v, --verbose              Enable verbose qemu."
	echo -e "  -d, --debug                Wait for gdb to connect to port 1234 before running."
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

	echo ""
}

while [[ "$#" -gt 0 ]]; do
	case $1 in
	-i | --image-location)
		IMAGE_LOCATION="$2"
		shift 2
		;;
	-v | --verbose)
		VERBOSE=true
		shift
		;;
	-d | --debug)
		DEBUG=true
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
	-machine q35
	-no-reboot
	-drive "format=raw,file=$IMAGE_LOCATION"
	-serial stdio
	-smp 1
	-usb
	-vga std
)
if [ "$VERBOSE" = true ]; then
	QEMU_OPTIONS+=(
		-d "int,cpu_reset"
	)
fi
if [ "$DEBUG" = true ]; then
	QEMU_OPTIONS+=(
		-s -S
	)
fi
echo -e "${BOLD}qemu-system-x86_64 ${QEMU_OPTIONS[*]}${NO_COLOR}"
qemu-system-x86_64 "${QEMU_OPTIONS[@]}"
