#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

YELLOW='\033[33m'
RED='\033[31m'
BOLD='\033[1m'
NO_COLOR='\033[0m'

BUILD_MODES=("Release" "Debug" "Profiling" "Fuzzing")
BUILD_MODE="${BUILD_MODES[0]}"
C_COMPILER=$(whereis clang-19 | awk '{ print $2 }')
LINKER=$(whereis ld | awk '{ print $2 }')
THREADS=$(grep -c ^processor /proc/cpuinfo)
THREADS=1
# ASSEMBLER=$(readlink -f ../dependencies/fasmg/fasmg.x64)
# ASSEMBLER_INCLUDE=$(readlink -f ../dependencies/fasmg/examples/x86/include/)

SELECTED_TARGETS=()

UNIT_TEST_BUILD=false
RUN_UNIT_TEST=false
INCLUDE_WHAT_YOU_USE=true
USE_AVX=true
USE_SSE=true

function display_usage() {
    echo -e "${RED}${BOLD}Usage: $0 [${YELLOW}OPTIONS${RED}]${NO_COLOR}"
    echo -e "${BOLD}Options:${NO_COLOR}"
    echo -e "  -m, --build-mode <TYPE>    Set the build mode (${YELLOW}${BUILD_MODES[*]}${NO_COLOR}). Default is ${YELLOW}${BUILD_MODES[0]}${NO_COLOR}."
    echo -e "  -u, --no-iwyu              Opt-out of include-what-you-use."
    echo -e "  -c, --c-compiler           Set the c-compiler. Default is ${YELLOW}${C_COMPILER}${NO_COLOR}."
    echo -e "  -s, --select-targets       Select specific target(s, space-separated) to be built."
    echo -e "  -t, --build-tests          Build tests."
    echo -e "  -r, --run-tests            Run tests."
    echo -e "  --no-avx                   Disable AVX. (This is applied to the operating system build only)"
    echo -e "  --no-sse                   Disable SSE. (This is applied to the operating system build only)"
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

    if [ "${#SELECTED_TARGETS[@]}" -gt 0 ]; then
        echo -n -e "${BOLD}${YELLOW}SELECTED_TARGETS${NO_COLOR}: ${YELLOW}"

        for ((i = 0; i < "${#SELECTED_TARGETS[@]}"; i++)); do
            local target="${SELECTED_TARGETS[$i]}"

            echo -n -e "${target}"
            if [ "$i" -lt $((${#SELECTED_TARGETS[@]} - 1)) ]; then
                echo -n -e " "
            fi
        done
        echo -e "${NO_COLOR}"
    else
        echo -e "${BOLD}${YELLOW}SELECTED_TARGETS${NO_COLOR}: ${YELLOW}ALL${NO_COLOR}"
    fi

    display_single_flag_configuration $UNIT_TEST_BUILD "Unit test build"
    display_single_flag_configuration $RUN_UNIT_TEST "Run unit tests"
    display_single_flag_configuration $INCLUDE_WHAT_YOU_USE "Include-What-You-Use"
    display_single_flag_configuration $USE_AVX "AVX"
    display_single_flag_configuration $USE_SSE "SSE"

    echo ""
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
    -u | --no-iwyu)
        INCLUDE_WHAT_YOU_USE=false
        shift
        ;;
    -c | --c-compiler)
        C_COMPILER="$2"
        shift 2
        ;;
    -s | --select-targets)
        shift
        while [[ "$#" -gt 0 && "$1" =~ ^[^-] ]]; do
            SELECTED_TARGETS+=("$1")
            shift
        done
        ;;
    -t | --build-tests)
        UNIT_TEST_BUILD=true
        shift
        ;;
    -r | --run-tests)
        RUN_UNIT_TEST=true
        shift
        ;;
    --no-avx)
        USE_AVX=false
        shift
        ;;
    --no-sse)
        USE_SSE=false
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

# -----------------------------------------------------------------------------
PROJECT="kernel/code"
echo -e "${BOLD}Going to build ${PROJECT} folder${NO_COLOR}"
cd ${PROJECT}

BUILD_DIRECTORY="build/"
if [ "$UNIT_TEST_BUILD" = true ]; then
    BUILD_DIRECTORY+="test/"
else
    BUILD_DIRECTORY+="prod/"
fi
BUILD_DIRECTORY+="${C_COMPILER##*/}/"

CONFIGURE_CMAKE_OPTIONS=(
    -S .
    -B "$BUILD_DIRECTORY"
    -D CMAKE_C_COMPILER="$C_COMPILER"
    --graphviz=target_dependencies.dot
    -D CMAKE_LINKER="$LINKER"
    -D CMAKE_BUILD_TYPE="$BUILD_MODE"
    -D USE_AVX="$USE_AVX"
    -D USE_SSE="$USE_SSE"
    -D UNIT_TEST_BUILD="$UNIT_TEST_BUILD"
    # 	-D CMAKE_ASM_COMPILER="$ASSEMBLER"
    # 	-D CMAKE_ASM_INCLUDE="$ASSEMBLER_INCLUDE"
)

if [ "$INCLUDE_WHAT_YOU_USE" = true ]; then
    CONFIGURE_CMAKE_OPTIONS+=(
        -D CMAKE_C_INCLUDE_WHAT_YOU_USE="include-what-you-use;-w;-Xiwyu;--no_default_mappings"
    )
fi

echo -e "${BOLD}cmake ${CONFIGURE_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${CONFIGURE_CMAKE_OPTIONS[@]}"

BUILD_CMAKE_OPTIONS=(--build "$BUILD_DIRECTORY" -j "${THREADS}")
if [ "${#SELECTED_TARGETS[@]}" -gt 0 ]; then
    BUILD_CMAKE_OPTIONS+=("--target")
    for target in "${SELECTED_TARGETS[@]}"; do
        BUILD_CMAKE_OPTIONS+=("${target}")
    done
fi

echo -e "${BOLD}cmake ${BUILD_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${BUILD_CMAKE_OPTIONS[@]}"

# Creating a symlink here so clangd understands. You have a good idea about
# splitting up builds so caches don't get polluted an you need to immediately
# patch something up.
find "$BUILD_DIRECTORY" -maxdepth 1 -name "compile_commands.json" -exec ln -f -s {} . \;

cd ../../

# -----------------------------------------------------------------------------
PROJECT="interoperation/code"
echo -e "${BOLD}Going to build ${PROJECT} folder${NO_COLOR} just for LSP purposes"
cd ${PROJECT}

CONFIGURE_CMAKE_OPTIONS=(
    -S .
    -B build/
    -D CMAKE_C_COMPILER="$C_COMPILER"
    -D CMAKE_LINKER="$LINKER"
    -D CMAKE_BUILD_TYPE="$BUILD_MODE"
    -D USE_AVX="$USE_AVX"
    -D USE_SSE="$USE_SSE"
    # 	-D CMAKE_ASM_COMPILER="$ASSEMBLER"
    # 	-D CMAKE_ASM_INCLUDE="$ASSEMBLER_INCLUDE"
)

if [ "$INCLUDE_WHAT_YOU_USE" = true ]; then
    CONFIGURE_CMAKE_OPTIONS+=(
        -D CMAKE_C_INCLUDE_WHAT_YOU_USE="include-what-you-use;-w;-Xiwyu;--no_default_mappings"
    )
fi

echo -e "${BOLD}cmake ${CONFIGURE_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${CONFIGURE_CMAKE_OPTIONS[@]}"

BUILD_CMAKE_OPTIONS=(--build build/ -j "${THREADS}")
echo -e "${BOLD}cmake ${BUILD_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${BUILD_CMAKE_OPTIONS[@]}"

cd ../../

if [ "$RUN_UNIT_TEST" = true ]; then
    if [ "${#SELECTED_TARGETS[@]}" -gt 0 ]; then
        # I am skill-deficient in bash, forgive me
        FILES_TO_EXECUTE=()
        for target in "${SELECTED_TARGETS[@]}"; do
            while IFS= read -r -d '' file; do
                FILES_TO_EXECUTE+=("$file")
            done < <(find kernel -executable -type f -name "*$target*" -print0)
        done

        for file in "${FILES_TO_EXECUTE[@]}"; do
            "$file"
        done
    else
        find kernel/code/build -executable -name "*-tests-${BUILD_MODE}*" -type f -exec {} \;
    fi

    exit 0
fi

# -----------------------------------------------------------------------------
PROJECT="uefi-image-creator/code"
echo -e "${BOLD}Going to build ${PROJECT} folder${NO_COLOR}"
cd ${PROJECT}

CONFIGURE_CMAKE_OPTIONS=(
    -S .
    -B build/
    -D CMAKE_C_COMPILER="$C_COMPILER"
    -D CMAKE_LINKER="$LINKER"
    -D CMAKE_BUILD_TYPE="$BUILD_MODE"
)

if [ "$INCLUDE_WHAT_YOU_USE" = true ]; then
    CONFIGURE_CMAKE_OPTIONS+=(
        -D CMAKE_C_INCLUDE_WHAT_YOU_USE="include-what-you-use;-w;-Xiwyu;"
    )
fi

echo -e "${BOLD}cmake ${CONFIGURE_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${CONFIGURE_CMAKE_OPTIONS[@]}"

BUILD_CMAKE_OPTIONS=(--build build/ -j "${THREADS}")
echo -e "${BOLD}cmake ${BUILD_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${BUILD_CMAKE_OPTIONS[@]}"

cd ../../

# -----------------------------------------------------------------------------
PROJECT="uefi/code"
echo -e "${BOLD}Going to build ${PROJECT} folder${NO_COLOR}"
cd ${PROJECT}

CONFIGURE_CMAKE_OPTIONS=(
    -S .
    -B build/
    -D CMAKE_C_COMPILER="$C_COMPILER"
    -D CMAKE_LINKER="$LINKER"
    -D USE_AVX="$USE_AVX"
    -D USE_SSE="$USE_SSE"
    #	-D CMAKE_ASM_COMPILER="$ASSEMBLER"
    # -D CMAKE_ASM_INCLUDE="$ASSEMBLER_INCLUDE"
    -D CMAKE_BUILD_TYPE="$BUILD_MODE"
)

if [ "$INCLUDE_WHAT_YOU_USE" = true ]; then
    CONFIGURE_CMAKE_OPTIONS+=(
        -D CMAKE_C_INCLUDE_WHAT_YOU_USE="include-what-you-use;-w;-Xiwyu;--no_default_mappings"
    )
fi

echo -e "${BOLD}cmake ${CONFIGURE_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${CONFIGURE_CMAKE_OPTIONS[@]}"

BUILD_CMAKE_OPTIONS=(--build build/ -j "${THREADS}")
echo -e "${BOLD}cmake ${BUILD_CMAKE_OPTIONS[*]}${NO_COLOR}"
cmake "${BUILD_CMAKE_OPTIONS[@]}"

cd ../../
