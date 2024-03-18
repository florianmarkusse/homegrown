#!/bin/bash
set -eo pipefail

YELLOW='\033[33m'
BLUE='\e[1;34m'
GREEN='\e[1;32m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

OS_NAME="testos"
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
echo -e "${BOLD}Install directory:  ${YELLOW}${PREFIX}${NO_COLOR}"
echo -e "${BOLD}Target:             ${YELLOW}${TARGET}${NO_COLOR}"
echo -e "${BOLD}================================${NO_COLOR}"

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
echo -e "${BOLD}Installing ${YELLOW}ovmf${NO_COLOR}"
sudo apt install -y ovmf

TARGET_TRIPLET="${TARGET}-${OS_NAME}-elf"

export PREFIX="${PREFIX}"
export TARGET="${TARGET}"
export PATH="${PREFIX}/bin:${PATH}"

DEPENDENCIES_DIR="dependencies"
echo -e "${BOLD}Creating ${YELLOW}${DEPENDENCIES_DIR}${NO_COLOR}${BOLD} directory${NO_COLOR}"
mkdir -p dependencies && cd dependencies

function is_present_or_download() {
	local directory="$1"
	local download_directory="$2"
	local download_file="$3"

	if [ -d "${directory}" ]; then
		echo -e "${BOLD}Requested ${YELLOW}${directory}${NO_COLOR}${BOLD} is already downloaded."
	else
		echo -e "${BOLD}Downloading ${YELLOW}${download_file}${NO_COLOR}"
		wget "${download_directory}/${download_file}"
		tar -xzf "${download_file}" && rm "${download_file}"
	fi
}

function is_target_installed() {
	local build_directory="$1"
	if [ -d "${build_directory}" ]; then
		echo -e "${BOLD}Requested ${YELLOW}${build_directory}${NO_COLOR}${BOLD} is already built."
		return 1
	fi
	echo -e "${BOLD}Installing ${YELLOW}${build_directory}${NO_COLOR}"
	return 0
}

function git_install_or_pull() {
	local repo=$1
	local folder=$2

	git clone "${repo}" 2>/dev/null || (cd "${folder}" && git pull && cd ../)
}

BINUTILS_VERSION="2.42"
BINUTILS="binutils-${BINUTILS_VERSION}"
BINUTILS_FILE="${BINUTILS}.tar.gz"
BUILD_BINUTILS="build-${BINUTILS}-${TARGET}"

is_present_or_download "${BINUTILS}" "https://ftp.gnu.org/gnu/binutils" "${BINUTILS_FILE}"
if is_target_installed "${BUILD_BINUTILS}"; then
	mkdir -p "${BUILD_BINUTILS}" && cd "${BUILD_BINUTILS}"
	../${BINUTILS}/configure --target="${TARGET_TRIPLET}" --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
	make
	make install
	cd ../
fi

GDB_VERSION="14.1"
GDB="gdb-${GDB_VERSION}"
GDB_FILE="${GDB}.tar.gz"
BUILD_GDB="build-${GDB}-${TARGET}"

is_present_or_download "${GDB}" "https://ftp.gnu.org/gnu/gdb" "${GDB_FILE}"
if is_target_installed "${BUILD_GDB}"; then
	mkdir -p "${BUILD_GDB}" && cd "${BUILD_GDB}"
	../${GDB}/configure --target="${TARGET_TRIPLET}" --prefix="$PREFIX" --disable-werrormake
	make all-gdb
	make install-gdb
	cd ../
fi

GCC_VERSION="13.2.0"
GCC="gcc-${GCC_VERSION}"
GCC_FILE="${GCC}.tar.gz"
BUILD_GCC="build-${GCC}-${TARGET}"

is_present_or_download "${GCC}" "https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}" "${GCC_FILE}"
if is_target_installed "${BUILD_GCC}"; then
	mkdir -p "${BUILD_GCC}" && cd "${BUILD_GCC}"
	"../${GCC}/configure" --target="${TARGET_TRIPLET}" --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
	make -j 8 all-gcc
	make -j 8 all-target-libgcc
	make install-gcc
	make install-target-libgcc
	cd ../
fi

FASMG_FILE="fasmg.kcm8.zip"
FASMG_DIR="fasmg"

if [ -d "$FASMG_DIR" ]; then
	echo -e "${BOLD}Requested ${YELLOW}${FASMG_DIR}${NO_COLOR}${BOLD} is already downloaded."
else
	echo -e "${BOLD}Downloading ${YELLOW}${FASMG_FILE}${NO_COLOR}"
	wget "https://flatassembler.net/${FASMG_FILE}"
	unzip "${FASMG_FILE}" -d fasmg && rm "${FASMG_FILE}"
fi

MKGPT="mkgpt"
git_install_or_pull git@github.com:jncronin/${MKGPT}.git ${MKGPT}
cd $MKGPT
automake --add-missing
autoreconf
./configure
make
sudo make install
cd ../

GNU_EFI="gnu-efi"
git_install_or_pull git@github.com:rhboot/${GNU_EFI}.git ${GNU_EFI}
cd $GNU_EFI
C_COMPILER=$(whereis x86_64-testos-elf-gcc | awk '{ print $2 }')
LINKER=$(whereis x86_64-testos-elf-ld | awk '{ print $2 }')
make ARCH=x86_64 CC="${C_COMPILER}" LD="${LINKER}"
cd ../

echo -e "${BOLD}${GREEN}Dependencies correctly installed!${NO_COLOR}"
echo -e "${BOLD}${BLUE}The journey begins...${NO_COLOR}"
