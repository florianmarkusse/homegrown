#!/bin/bash
set -eo pipefail

YELLOW='\033[33m'
BLUE='\e[1;34m'
GREEN='\e[1;32m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

TARGETS=("x86_64" "arm")
TARGET="${TARGETS[0]}"

function display_usage() {
	echo -e "Usage: $0 [${YELLOW}OPTIONS${NO_COLOR}]"
	echo -e "${BOLD}Options:${NO_COLOR}"
	echo -e "  -t, --target <TYPE>        Set the target (${YELLOW}${TARGETS[*]}${NO_COLOR}). Default is ${YELLOW}${TARGETS[0]}${NO_COLOR}."
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
	-t | --target)
		if ! is_valid_target "$2"; then
			echo -e "${RED}${BOLD}Invalid ${YELLOW}Target${NO_COLOR}. Valid options: ${NO_COLOR}${YELLOW}${TARGETS[*]}${NO_COLOR}."
			exit 1
		fi
		TARGET="$2"
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

PREFIX="${HOME}/opt/cross/${TARGET}"

echo -e "${BOLD}Configuration...${NO_COLOR}"
echo -e "${BOLD}================================${NO_COLOR}"
echo -e "${BOLD}Install folder: ${YELLOW}${PREFIX}${NO_COLOR}"
echo -e "${BOLD}Target:         ${YELLOW}${TARGET}${NO_COLOR}"
echo -e "${BOLD}================================${NO_COLOR}"

exit 0

cd "$(dirname "${BASH_SOURCE[0]}")"

echo -e "${BOLD}Creating ${YELLOW}${PREFIX}${NO_COLOR}"
mkdir -p "${PREFIX}"

echo -e "${BOLD}Installing ${YELLOW}makeinfo${NO_COLOR}"
sudo apt install -y texinfo
echo -e "${BOLD}Installing ${YELLOW}xorriso${NO_COLOR}"
sudo apt install -y xorriso
echo -e "${BOLD}Installing ${YELLOW}mtools${NO_COLOR}"
sudo apt install -y mtools
echo -e "${BOLD}Installing ${YELLOW}qemu${NO_COLOR}"
sudo apt install -y qemu
echo -e "${BOLD}Installing ${YELLOW}qemu-system-x86${NO_COLOR}"
sudo apt install -y qemu-system-x86
echo -e "${BOLD}Installing ${YELLOW}nasm${NO_COLOR}"
sudo apt install -y nasm

export PREFIX="${PREFIX}"
export TARGET="${TARGET}"
export PATH="${PREFIX}/bin:${PATH}"

DEPENDENCIES_DIR="dependencies"
echo -e "${BOLD}Creating ${YELLOW}${DEPENDENCIES_DIR}${NO_COLOR}${BOLD} directory${NO_COLOR}"
mkdir -p dependencies && cd dependencies

BINUTILS_VERSION="2.42"
BINUTILS="binutils-${BINUTILS_VERSION}"
BINUTILS_FILE="${BINUTILS}.tar.gz"
BUILD_BINUTILS="build-${BINUTILS}-${TARGET}"

if [ -d "${BUILD_BINUTILS}" ]; then
	echo -e "${BOLD}Requested ${YELLOW}${BINUTILS}${NO_COLOR}${BOLD} is already built."
else
	echo -e "${BOLD}Downloading ${YELLOW}${BINUTILS}${NO_COLOR}"
	wget "https://ftp.gnu.org/gnu/binutils/${BINUTILS_FILE}"
	tar -xzf "${BINUTILS_FILE}" && rm "${BINUTILS_FILE}"
fi

if [ -d "${BUILD_BINUTILS}" ]; then
	echo -e "${BOLD}Requested ${YELLOW}${BINUTILS}${NO_COLOR}${BOLD} is already built."
else
	echo -e "${BOLD}Requested ${YELLOW}${BINUTILS}${NO_COLOR}${BOLD} is already built."
fi
echo -e "${BOLD}Installing ${YELLOW}${BINUTILS}${NO_COLOR}"
mkdir -p "${BUILD_BINUTILS}" && cd "${BUILD_BINUTILS}"
../${BINUTILS}/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ../

GDB_VERSION="14.1"
GDB="gdb-${GDB_VERSION}"
GDB_FILE="${GDB}.tar.gz"
BUILD_GDB="build-${GDB}-${TARGET}"

echo -e "${BOLD}Downloading ${YELLOW}${GDB}${NO_COLOR}"
wget "https://ftp.gnu.org/gnu/gdb/${GDB_FILE}"
tar -xzf "${GDB_FILE}" && rm "${GDB_FILE}"

echo -e "${BOLD}Installing ${YELLOW}${GDB}${NO_COLOR}"
mkdir -p "${BUILD_GDB}" && cd "${BUILD_GDB}"
../${GDB}/configure --target="$TARGET" --prefix="$PREFIX" --disable-werrormake
make all-gdb
make install-gdb
cd ../

GCC_VERSION="13.2.0"
GCC="gcc-${GCC_VERSION}"
GCC_FILE="${GCC}.tar.gz"
BUILD_GCC="build-${GCC}-${TARGET}"

echo -e "${BOLD}Downloading ${YELLOW}${GCC}${NO_COLOR}"
wget "https://ftp.gnu.org/gnu/gcc/${GCC}/${GCC_FILE}"
tar -xzf "${GCC_FILE}" && rm "${GCC_FILE}"

echo -e "${BOLD}Installing ${YELLOW}${GCC}${NO_COLOR}"
mkdir -p "${BUILD_GCC}" && cd "${BUILD_GCC}"
"../${GCC}/configure" --target="$TARGET" --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make -j 8 all-gcc
make -j 8 all-target-libgcc
make install-gcc
make install-target-libgcc
cd ../

echo -e "${BOLD}${GREEN}Dependencies correctly installed!${NO_COLOR}"
echo -e "${BOLD}${BLUE}The journey begins...${NO_COLOR}"
