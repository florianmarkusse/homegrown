#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

YELLOW='\033[33m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

BUILD_MODES=("Release" "Debug" "Profiling" "Fuzzing")
BUILD_MODE="${BUILD_MODES[0]}"
RUN_TESTS=false

function display_usage() {
    echo -e "${RED}${BOLD}Usage: $0 [${YELLOW}OPTIONS${RED}]${NO_COLOR}"
    echo -e "${BOLD}Options:${NO_COLOR}"
    echo -e "  -m, --build-mode <TYPE>    Set the build mode (${YELLOW}${BUILD_MODES[*]}${NO_COLOR}). Default is ${YELLOW}${BUILD_MODES[0]}${NO_COLOR}."
    echo -e "  -t, --run-tests            Run the built tests."
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

    echo -e "${BOLD}${YELLOW}BUILD_MODE${NO_COLOR}: ${YELLOW}${BUILD_MODE}${NO_COLOR}"

    display_single_flag_configuration $RUN_TESTS "Tests"

    echo ""
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
    -t | --run-tests)
        RUN_TESTS=true
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

PROJECT="kernel-test/code"
echo -e "${BOLD}Going to build ${PROJECT} folder${NO_COLOR}"
cd ${PROJECT}

CONFIGURE_CMAKE_OPTIONS=(
    -S .
    -B build/
    -D CMAKE_C_COMPILER="clang"
    -D CMAKE_LINKER="clang"
    -D CMAKE_BUILD_TYPE="$BUILD_MODE"
)

if [ "$INCLUDE_WHAT_YOU_USE" = true ]; then
    CONFIGURE_CMAKE_OPTIONS+=(
        -D CMAKE_C_INCLUDE_WHAT_YOU_USE="include-what-you-use;-w;-Xiwyu;"
    )
fi

echo -e "${BOLD}cmake ${CONFIGURE_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${CONFIGURE_CMAKE_OPTIONS[@]}"

BUILD_CMAKE_OPTIONS=(--build build/)
echo -e "${BOLD}cmake ${BUILD_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${BUILD_CMAKE_OPTIONS[@]}"

if [ "$RUN_TESTS" = true ]; then
    find "build/" -name "*-${BUILD_MODE}" -type f -executable -execdir {} \;
fi

cd ../../
