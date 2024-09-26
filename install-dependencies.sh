#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

BLUE='\033[34m'
GREEN='\033[32m'
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

echo -e "${BOLD}Installing ${YELLOW}go${NO_COLOR}"
echo -e "${BOLD}Removing previous ${YELLOW}go${NO_COLOR}${BOLD} installation (if any)..."
sudo rm -rf /usr/local/go

LATEST_VERSION=$(curl -s https://go.dev/VERSION?m=text | head -n 1 | sed s/go//)
if [ -z "$LATEST_VERSION" ]; then
    echo -e "${BOLD}Unable to fetch the latest ${YELLOW}go${NO_COLOR}${BOLD} version."
    exit 1
fi

echo -e "${BOLD}Installing ${YELLOW}go${NO_COLOR}${BOLD} ${LATEST_VERSION}"
GO_TARBALL="go${LATEST_VERSION}.linux-amd64.tar.gz"
DOWNLOAD_URL="https://go.dev/dl/${GO_TARBALL}"
wget "${DOWNLOAD_URL}" -O "/tmp/${GO_TARBALL}" || (echo -e "${BOLD}Failed to download ${YELLOW}go${NO_COLOR}${BOLD}." && exit 1)
sudo tar -xzf "/tmp/${GO_TARBALL}" -C /usr/local || (echo -e "${BOLD}Failed to extract ${YELLOW}go${NO_COLOR}${BOLD}." && exit 1)
rm -f "/tmp/${GO_TARBALL}"

echo -e "${BOLD}Adding ${YELLOW}go${NO_COLOR}${BOLD} to your system PATH..."
if ! grep -q "export PATH=$PATH:/usr/local/go/bin" ~/.bashrc; then
    echo "export PATH=$PATH:/usr/local/go/bin" >>~/.bashrc
fi

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
echo -e "${BOLD}${GREEN}Please reload terminal to update PATH changes!${NO_COLOR}"
echo -e "${BOLD}${BLUE}The journey begins...${NO_COLOR}"
