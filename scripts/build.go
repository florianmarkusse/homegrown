package main

import (
	"fmt"
	"os"
	"path/filepath"
	"scripts/common"
	"strings"
)

const BUILD_MODE_LONG_FLAG = "build-mode"
const BUILD_MODE_SHORT_FLAG = "m"

const NO_IWYU_LONG_FLAG = "no-iwyu"
const NO_IWYU_SHORT_FLAG = "u"

const C_COMPILER_LONG_FLAG = "c-compiler"
const C_COMPILER_SHORT_FLAG = "c"

const SELECT_TARGETS_LONG_FLAG = "select-targets"
const SELECT_TARGETS_SHORT_FLAG = "s"

const BUILD_TESTS_LONG_FLAG = "build-tests"
const BUILD_TESTS_SHORT_FLAG = "t"

const RUN_TESTS_LONG_FLAG = "run-tests"
const RUN_TESTS_SHORT_FLAG = "r"

const NO_AVX_LONG_FLAG = "no-avx"

const NO_SSE_LONG_FLAG = "no-sse"

const HELP_LONG_FLAG = "help"
const HELP_SHORT_FLAG = "h"

const EXIT_SUCCESS = 0
const EXIT_GENERAL_ERROR = 1
const EXIT_MISSING_ARGUMENT = 2

var possibleBuildModes = [...]string{"Release", "Debug", "Profiling", "Fuzzing"}
var buildMode = possibleBuildModes[0]

var cCompiler = "clang-19"

func getBuildModes() string {
	buildString := strings.Builder{}
	buildString.WriteString(common.YELLOW)
	for i, mode := range possibleBuildModes {
		buildString.WriteString(mode)
		if i != len(possibleBuildModes)-1 {
			buildString.WriteString(" ")
		}
	}
	buildString.WriteString(common.RESET)

	return buildString.String()
}

func usage() {
	common.DisplayUsage()
	fmt.Printf("  %s %s[OPTIONS]%s\n", filepath.Base(os.Args[0]), common.GRAY, common.RESET)
	fmt.Printf("\n")
	common.DisplayOptionalFlags()

	var buildModeDescription = fmt.Sprintf("Set the build mode (%s), default is %s%s%s%s", getBuildModes(), common.BOLD, common.YELLOW, buildMode, common.RESET)
	common.DisplayArgumentInput(BUILD_MODE_SHORT_FLAG, BUILD_MODE_LONG_FLAG, buildModeDescription)
	var cCompilerDescription = fmt.Sprintf("Set the c-compiler, default is %s%s%s%s", common.BOLD, common.YELLOW, cCompiler, common.RESET)
	common.DisplayArgumentInput(C_COMPILER_SHORT_FLAG, C_COMPILER_LONG_FLAG, cCompilerDescription)
	// common.DisplayArgumentInput(DEBUG_SHORT_FLAG, DEBUG_LONG_FLAG, "Wait for gdb to connect to port 1234 before running")
	// common.DisplayArgumentInput(HELP_SHORT_FLAG, HELP_LONG_FLAG, "Display this help message")
	// fmt.Printf("%sOptional%s Options:\n", common.BOLD, common.RESET)
	// common.DisplayArgumentInput(VERBOSE_SHORT_FLAG, VERBOSE_LONG_FLAG, "Enable verbose QEMU")
	// common.DisplayArgumentInput(DEBUG_SHORT_FLAG, DEBUG_LONG_FLAG, "Wait for gdb to connect to port 1234 before running")
	// common.DisplayArgumentInput(HELP_SHORT_FLAG, HELP_LONG_FLAG, "Display this help message")
	// fmt.Printf("%s%sExit codes%s:\n", common.CYAN, common.BOLD, common.RESET)
	// common.DisplayExitCode(EXIT_SUCCESS, "Success")
	// common.DisplayExitCode(EXIT_MISSING_ARGUMENT, "Missing argument(s)")
	// fmt.Printf("\n")
	// fmt.Printf("\n")
	// fmt.Printf("%s%sExamples%s:\n", common.BLUE, common.BOLD, common.RESET)
	// fmt.Printf("  %s --%s test.hdd --%s bios.bin\n", filepath.Base(os.Args[0]), UEFI_LOCATION_LONG_FLAG, OS_LOCATION_LONG_FLAG)
	// fmt.Printf("  %s -%s=test.hdd -%s bios.bin -%s --%s\n", filepath.Base(os.Args[0]), UEFI_LOCATION_SHORT_FLAG, OS_LOCATION_LONG_FLAG, VERBOSE_SHORT_FLAG, DEBUG_LONG_FLAG)
	// fmt.Printf("\n")
}

func main() {
	usage()

}
