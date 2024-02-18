#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

YELLOW='\033[33m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

IMAGE_LOCATION_DEFAULT="bootboot-out/mykernel.iso"
IMAGE_LOCATION=${1:-"${IMAGE_LOCATION_DEFAULT}"}

function display_usage() {
	echo -e "Usage: $0 [${YELLOW}OPTIONS${NO_COLOR}]"
	echo -e "${BOLD}Options:${NO_COLOR}"
	echo -e "  -i, --image-location       Set the .iso location. Default is ${YELLOW}${IMAGE_LOCATION_DEFAULT}${NO_COLOR}."
	echo -e "  -h, --help                 Display this help message."
	exit 1
}

while [[ "$#" -gt 0 ]]; do
	case $1 in
	-i | --image-location)
		IMAGE_LOCATION="$2"
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

qemu-system-x86_64 \
	-accel tcg,thread=single \
	-cpu core2duo \
	-m 128 \
	-no-reboot \
	-drive format=raw,media=cdrom,file="${IMAGE_LOCATION}" \
	-serial stdio \
	-smp 1 \
	-usb \
	-vga std
