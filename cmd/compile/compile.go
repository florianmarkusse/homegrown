package main

import (
	"cmd/common"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

const BUILD_MODE_LONG_FLAG = "build-mode"
const BUILD_MODE_SHORT_FLAG = "m"

const IWYU_LONG_FLAG = "iwyu"
const IWYU_SHORT_FLAG = "u"

const C_COMPILER_LONG_FLAG = "c-compiler"
const C_COMPILER_SHORT_FLAG = "c"

const LINKER_LONG_FLAG = "linker"
const LINKER_SHORT_FLAG = "l"

const SELECT_TARGETS_LONG_FLAG = "targets"
const SELECT_TARGETS_SHORT_FLAG = "s"

const TEST_BUILD_LONG_FLAG = "test-build"
const TEST_BUILD_SHORT_FLAG = "t"

const RUN_TESTS_LONG_FLAG = "run-tests"
const RUN_TESTS_SHORT_FLAG = "r"

const THREADS_LONG_FLAG = "threads"

const AVX_LONG_FLAG = "avx"

const SSE_LONG_FLAG = "sse"

const HELP_LONG_FLAG = "help"
const HELP_SHORT_FLAG = "h"

const EXIT_SUCCESS = 0
const EXIT_MISSING_ARGUMENT = 1

func usage() {
	common.DisplayUsage()
	fmt.Printf("  %s %s[OPTIONS]%s\n", filepath.Base(os.Args[0]), common.GRAY, common.RESET)
	fmt.Printf("\n")
	common.DisplayOptionalFlags()

	// Not sure why go doesnt understand string lengths of this one, but whatever
	var buildModeDescription = fmt.Sprintf("Set the build mode (%s)        ", getBuildModes())
	common.DisplayArgumentInput(BUILD_MODE_SHORT_FLAG, BUILD_MODE_LONG_FLAG, buildModeDescription, buildMode)

	common.DisplayArgumentInput(IWYU_SHORT_FLAG, IWYU_LONG_FLAG, "Set include-what-you-use", fmt.Sprint(includeWhatYouUse))

	common.DisplayArgumentInput(C_COMPILER_SHORT_FLAG, C_COMPILER_LONG_FLAG, "Set the c-compiler", fmt.Sprint(cCompiler))

	common.DisplayArgumentInput(LINKER_SHORT_FLAG, LINKER_LONG_FLAG, "Set the linker", fmt.Sprint(linker))

	common.DisplayArgumentInput(SELECT_TARGETS_SHORT_FLAG, SELECT_TARGETS_LONG_FLAG, "Select specific target(s, comma-separated) to be built", "_all_")

	common.DisplayArgumentInput(TEST_BUILD_SHORT_FLAG, TEST_BUILD_LONG_FLAG, "Build for tests", fmt.Sprint(testBuild))

	common.DisplayArgumentInput(RUN_TESTS_SHORT_FLAG, RUN_TESTS_LONG_FLAG, "Run tests", fmt.Sprint(runTests))

	common.DisplayLongFlagArgumentInput(THREADS_LONG_FLAG, "Set the number of threads to use for compiling", fmt.Sprint(threads))

	common.DisplayLongFlagArgumentInput(AVX_LONG_FLAG, "Set SSE", fmt.Sprint(useSSE))

	common.DisplayLongFlagArgumentInput(SSE_LONG_FLAG, "Set AVX", fmt.Sprint(useAVX))

	common.DisplayNoDefaultArgumentInput(HELP_SHORT_FLAG, HELP_LONG_FLAG, "Display this help message")
	fmt.Printf("\n")
	common.DisplayExitCodes()
	common.DisplayExitCode(EXIT_SUCCESS, "Success")
	common.DisplayExitCode(EXIT_MISSING_ARGUMENT, "Incorrect argument(s)")
	fmt.Printf("\n")
	common.DisplayExamples()
	fmt.Printf("  %s\n", filepath.Base(os.Args[0]))
	fmt.Printf("  %s -%s=%s -%s=%t --%s text,log --%s -%s\n", filepath.Base(os.Args[0]),
		BUILD_MODE_LONG_FLAG, possibleBuildModes[1], IWYU_LONG_FLAG, false, SELECT_TARGETS_LONG_FLAG, TEST_BUILD_LONG_FLAG, RUN_TESTS_SHORT_FLAG)
	fmt.Printf("\n")
}

var possibleBuildModes = [...]string{"Release", "Debug", "Profiling", "Fuzzing"}
var buildMode = possibleBuildModes[0]
var includeWhatYouUse = true
var cCompiler = "clang-19"
var linker = "ld"
var targets string
var testBuild = false
var runTests = false
var threads = runtime.NumCPU()
var useAVX = true
var useSSE = true
var help = false

func getBuildModes() string {
	buildString := strings.Builder{}
	buildString.WriteString(common.WHITE)
	for i, mode := range possibleBuildModes {
		buildString.WriteString(mode)
		if i != len(possibleBuildModes)-1 {
			buildString.WriteString(" ")
		}
	}
	buildString.WriteString(common.RESET)

	return buildString.String()
}

func main() {

	flag.StringVar(&buildMode, BUILD_MODE_LONG_FLAG, buildMode, "")
	flag.StringVar(&buildMode, BUILD_MODE_SHORT_FLAG, buildMode, "")

	flag.BoolVar(&includeWhatYouUse, IWYU_LONG_FLAG, includeWhatYouUse, "")
	flag.BoolVar(&includeWhatYouUse, IWYU_SHORT_FLAG, includeWhatYouUse, "")

	flag.StringVar(&cCompiler, C_COMPILER_LONG_FLAG, cCompiler, "")
	flag.StringVar(&cCompiler, C_COMPILER_SHORT_FLAG, cCompiler, "")

	flag.StringVar(&linker, LINKER_LONG_FLAG, linker, "")
	flag.StringVar(&linker, LINKER_SHORT_FLAG, linker, "")

	flag.StringVar(&targets, SELECT_TARGETS_LONG_FLAG, "", "")
	flag.StringVar(&targets, SELECT_TARGETS_SHORT_FLAG, "", "")

	flag.BoolVar(&testBuild, TEST_BUILD_LONG_FLAG, testBuild, "")
	flag.BoolVar(&testBuild, TEST_BUILD_SHORT_FLAG, testBuild, "")

	flag.BoolVar(&runTests, RUN_TESTS_LONG_FLAG, runTests, "")
	flag.BoolVar(&runTests, RUN_TESTS_SHORT_FLAG, runTests, "")

	flag.IntVar(&threads, THREADS_LONG_FLAG, threads, "")

	flag.BoolVar(&useAVX, AVX_LONG_FLAG, useAVX, "")

	flag.BoolVar(&useSSE, SSE_LONG_FLAG, useSSE, "")

	flag.BoolVar(&help, HELP_LONG_FLAG, help, "")
	flag.BoolVar(&help, HELP_SHORT_FLAG, help, "")

	flag.Usage = usage
	flag.Parse()

	var showHelpAndExit = false

	var correctBuildMode = false
	for _, mode := range possibleBuildModes {
		if buildMode == mode {
			correctBuildMode = true
		}
	}
	if !correctBuildMode {
		showHelpAndExit = true
	}

	if help {
		showHelpAndExit = true
	}

	if showHelpAndExit {
		usage()
		if help {
			os.Exit(EXIT_SUCCESS)
		} else {
			os.Exit(EXIT_MISSING_ARGUMENT)
		}
	}

	fmt.Println(targets)
	var selectedTargets = strings.FieldsFunc(targets, func(r rune) bool {
		return r == ','
	})

	for _, element := range selectedTargets {
		fmt.Println(element)
	}

	fmt.Println("You made it here!!!!!!!!!!!!!!!!")
}
