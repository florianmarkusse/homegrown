#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

YELLOW='\033[33m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

OS_LOCATION=""
UEFI_LOCATION=""
VERBOSE=false
DEBUG=false

function display_usage() {
    echo -e "Usage: $0 [${YELLOW}OPTIONS${NO_COLOR}]"
    echo -e "${BOLD}Options:${NO_COLOR}"
    echo -e "  -o, --os-location          ${BOLD}${YELLOW}Required.${NO_COLOR} Set the OS (.hdd) location."
    echo -e "  -u, --uefi-location        ${BOLD}${YELLOW}Required.${NO_COLOR} Set the UEFI (.bin) location to emulate UEFI environment (like OVMF firmware)"
    echo -e "  -v, --verbose              Enable verbose qemu."
    echo -e "  -d, --debug                Wait for gdb to connect to port 1234 before running."
    echo -e "  -h, --help                 Display this help message."
    exit 1
}

function display_single_flag_configuration() {
    local flag="$1"
    local option="$2"

    display_single_flag_configuration ${VERBOSE} "Verbose"
    display_single_flag_configuration ${DEBUG} "Debug"

    if [ "$flag" = true ]; then
        echo -e "${BOLD}${YELLOW}${option}${NO_COLOR}: ${YELLOW}Yes${NO_COLOR}"
    else
        echo -e "${BOLD}${YELLOW}${option}${NO_COLOR}: ${YELLOW}No${NO_COLOR}"
    fi
}

function display_configuration() {
    echo -e "${BOLD}${YELLOW}Configuration...${NO_COLOR}"

    echo -e "${BOLD}${YELLOW}OS location${NO_COLOR}: ${YELLOW}${OS_LOCATION}${NO_COLOR}"
    echo -e "${BOLD}${YELLOW}UEFI location${NO_COLOR}: ${YELLOW}${UEFI_LOCATION_LOCATION}${NO_COLOR}"

    echo ""
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
    -o | --os-location)
        OS_LOCATION="$2"
        shift 2
        ;;
    -u | --uefi-location)
        UEFI_LOCATION="$2"
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

[ -z "${UEFI_LOCATION}" ] || [ -z "${OS_LOCATION}" ] && display_usage && exit 1

QEMU_OPTIONS=(
    -m 512
    -machine q35
    -no-reboot
    -bios "${UEFI_LOCATION}"
    -drive "format=raw,file=$OS_LOCATION"
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
else
    # Virtualization and debugging does not work sadge https://forum.osdev.org/viewtopic.php?t=39998
    QEMU_OPTIONS+=(
        -cpu host
        -enable-kvm
    )
fi

echo -e "${BOLD}qemu-system-x86_64 ${QEMU_OPTIONS[*]}${NO_COLOR}"
qemu-system-x86_64 "${QEMU_OPTIONS[@]}"
