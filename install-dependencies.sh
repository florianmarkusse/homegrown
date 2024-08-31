#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

YELLOW='\033[33m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

function display_usage() {
    echo -e "Usage: $0 [${YELLOW}OPTIONS${NO_COLOR}]"
    echo -e "Script to install the necessary dependencies to build the project on this machine x86_64"
    echo -e "${BOLD}Options:${NO_COLOR}"
    echo -e "  -h, --help                 Display this help message."
    exit 1
}

function is_valid_target() {
    local target="$1"
    for valid_target in "${TARGETS[@]}"; do
        if [[ "${target}" == "${valid_target}" ]]; then
            return 0
        fi
    done
    return 1
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
    -h | --help)
        display_usage
        ;;
    *)
        display_usage
        ;;
    esac
done

LLVM_VERSION=19
echo -e "${BOLD}Installing ${YELLOW}llvm toolchain ${LLVM_VERSION}${NO_COLOR}"
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh "${LLVM_VERSION}"
rm ./llvm.sh

echo -e "${BOLD}Installing ${YELLOW}cmake${NO_COLOR}"
sudo apt install -y cmake
echo -e "${BOLD}Installing ${YELLOW}iwyu${NO_COLOR}"
sudo apt install -y iwyu
echo -e "${BOLD}Installing ${YELLOW}gcc${NO_COLOR}"
sudo apt install -y gcc
echo -e "${BOLD}Installing ${YELLOW}python3${NO_COLOR}"
sudo apt install -y python3
echo -e "${BOLD}Installing ${YELLOW}qemu${NO_COLOR}"
sudo apt install -y qemu
echo -e "${BOLD}Installing ${YELLOW}qemu-system-x86${NO_COLOR}"
sudo apt install -y qemu-system-x86
echo -e "${BOLD}Installing ${YELLOW}ovmf${NO_COLOR}"
sudo apt install -y ovmf
# This is the binary that emulates UEFI on qemu
cp /usr/share/ovmf/OVMF.fd bios.bin

# DEPENDENCIES_DIR="dependencies"
# echo -e "${BOLD}Creating ${YELLOW}${DEPENDENCIES_DIR}${NO_COLOR}${BOLD} directory${NO_COLOR}"
# mkdir -p dependencies && cd dependencies
#
# FASMG_FILE="fasmg.kcm8.zip"
# FASMG_DIR="fasmg"
#
# if [ -d "$FASMG_DIR" ]; then
#     echo -e "${BOLD}Requested ${YELLOW}${FASMG_DIR}${NO_COLOR}${BOLD} is already downloaded."
# else
#     echo -e "${BOLD}Downloading ${YELLOW}${FASMG_FILE}${NO_COLOR}"
#     wget "https://flatassembler.net/${FASMG_FILE}"
#     unzip "${FASMG_FILE}" -d fasmg && rm "${FASMG_FILE}"
# fi
#

echo -e "${BOLD}${GREEN}Dependencies correctly installed!${NO_COLOR}"
echo -e "${BOLD}${BLUE}The journey begins...${NO_COLOR}"
