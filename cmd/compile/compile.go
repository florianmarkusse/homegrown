package main

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/flags"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/bitfield/script"
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
const DEFAULT_TARGETS = "_ALL_"

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
const EXIT_CLI_PARSING_ERROR = 2

const PROJECT_FOLDER = "projects/"
const KERNEL_CODE_FOLDER = PROJECT_FOLDER + "kernel/code"

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

func arrayIntoPrintableString(array []string) string {
	builder := strings.Builder{}
	for i, element := range array {
		builder.WriteString(element)
		if i != len(array)-1 {
			builder.WriteString(" ")
		}
	}

	return builder.String()
}

func usage() {
	flags.DisplayUsage("")
	fmt.Printf("\n")
	flags.DisplayOptionalFlags()

	// Not sure why go doesnt understand string lengths of this one, but whatever
	var buildModeDescription = fmt.Sprintf("Set the build mode (%s%s%s)        ", common.WHITE,
		arrayIntoPrintableString(possibleBuildModes[:]), common.RESET)
	flags.DisplayArgumentInput(BUILD_MODE_SHORT_FLAG, BUILD_MODE_LONG_FLAG, buildModeDescription, buildMode)

	flags.DisplayArgumentInput(IWYU_SHORT_FLAG, IWYU_LONG_FLAG, "Set include-what-you-use", fmt.Sprint(includeWhatYouUse))

	flags.DisplayArgumentInput(C_COMPILER_SHORT_FLAG, C_COMPILER_LONG_FLAG, "Set the c-compiler", fmt.Sprint(cCompiler))

	flags.DisplayArgumentInput(LINKER_SHORT_FLAG, LINKER_LONG_FLAG, "Set the linker", fmt.Sprint(linker))

	flags.DisplayArgumentInput(SELECT_TARGETS_SHORT_FLAG, SELECT_TARGETS_LONG_FLAG, "Select specific target(s, comma-separated) to be built", DEFAULT_TARGETS)

	flags.DisplayArgumentInput(TEST_BUILD_SHORT_FLAG, TEST_BUILD_LONG_FLAG, "Build for tests", fmt.Sprint(testBuild))

	flags.DisplayArgumentInput(RUN_TESTS_SHORT_FLAG, RUN_TESTS_LONG_FLAG, "Run tests", fmt.Sprint(runTests))

	flags.DisplayLongFlagArgumentInput(THREADS_LONG_FLAG, "Set the number of threads to use for compiling", fmt.Sprint(threads))

	flags.DisplayLongFlagArgumentInput(AVX_LONG_FLAG, "Set SSE", fmt.Sprint(useSSE))

	flags.DisplayLongFlagArgumentInput(SSE_LONG_FLAG, "Set AVX", fmt.Sprint(useAVX))

	flags.DisplayNoDefaultArgumentInput(HELP_SHORT_FLAG, HELP_LONG_FLAG, "Display this help message")
	fmt.Printf("\n")
	flags.DisplayExitCodes()
	flags.DisplayExitCode(EXIT_SUCCESS, "Success")
	flags.DisplayExitCode(EXIT_MISSING_ARGUMENT, "Incorrect argument(s)")
	flags.DisplayExitCode(EXIT_CLI_PARSING_ERROR, "CLI parsing error")
	fmt.Printf("\n")
	flags.DisplayExamples()
	fmt.Printf("  %s\n", filepath.Base(os.Args[0]))
	fmt.Printf("  %s -%s=%s -%s=%t --%s text,log --%s -%s\n", filepath.Base(os.Args[0]),
		BUILD_MODE_LONG_FLAG, possibleBuildModes[1], IWYU_LONG_FLAG, false, SELECT_TARGETS_LONG_FLAG, TEST_BUILD_LONG_FLAG, RUN_TESTS_SHORT_FLAG)
	fmt.Printf("\n")
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

	var selectedTargets = strings.FieldsFunc(targets, func(r rune) bool {
		return r == ','
	})
	var usingTargets = len(selectedTargets) > 0

	configuration.DisplayConfiguration()
	configuration.DisplayStringArgument(BUILD_MODE_LONG_FLAG, buildMode)
	configuration.DisplayBoolArgument(IWYU_LONG_FLAG, includeWhatYouUse)
	configuration.DisplayStringArgument(C_COMPILER_LONG_FLAG, cCompiler)
	configuration.DisplayStringArgument(LINKER_LONG_FLAG, linker)

	var targetsConfiguration string
	if usingTargets {
		targetsConfiguration = arrayIntoPrintableString(selectedTargets[:])
	} else {
		targetsConfiguration = DEFAULT_TARGETS
	}
	configuration.DisplayStringArgument(SELECT_TARGETS_LONG_FLAG, targetsConfiguration)

	configuration.DisplayBoolArgument(TEST_BUILD_LONG_FLAG, testBuild)
	configuration.DisplayBoolArgument(RUN_TESTS_LONG_FLAG, runTests)
	configuration.DisplayIntArgument(THREADS_LONG_FLAG, threads)
	configuration.DisplayBoolArgument(AVX_LONG_FLAG, useAVX)
	configuration.DisplayBoolArgument(SSE_LONG_FLAG, useSSE)
	fmt.Printf("\n")

	fmt.Printf("%sGoing to build %s folder%s", common.BOLD, KERNEL_CODE_FOLDER, common.RESET)

	var configureOptions = make([]string, 0)

	configureOptions = append(configureOptions, fmt.Sprintf("-S %s", KERNEL_CODE_FOLDER))

	buildDirectory := strings.Builder{}
	buildDirectory.WriteString(fmt.Sprintf("%s/", KERNEL_CODE_FOLDER))
	buildDirectory.WriteString("build/")
	if testBuild {
		buildDirectory.WriteString("test/")
	} else {
		buildDirectory.WriteString("prod/")
	}
	buildDirectory.WriteString(fmt.Sprintf("%s/", cCompiler))
	configureOptions = append(configureOptions, fmt.Sprintf("-B %s", buildDirectory.String()))

	configureOptions = append(configureOptions, fmt.Sprintf("-D CMAKE_C_COMPILER=%s", cCompiler))
	configureOptions = append(configureOptions, "--graphviz=dependency-graph.dot")
	configureOptions = append(configureOptions, fmt.Sprintf("-D CMAKE_BUILD_TYPE=%s", buildMode))

	configureOptions = append(configureOptions, fmt.Sprintf("-D USE_AVX=%s", useAVX))
	configureOptions = append(configureOptions, fmt.Sprintf("-D USE_SSE=%s", useSSE))
	configureOptions = append(configureOptions, fmt.Sprintf("-D UNIT_TEST_BUILD=%s", testBuild))

	if includeWhatYouUse {
		configureOptions = append(configureOptions, "-D CMAKE_C_INCLUDE_WHAT_YOU_USE=\"include-what-you-use;-w;-Xiwyu;--no_default_mappings\"")
	}

	var exec = fmt.Sprintf("ls %s", KERNEL_CODE_FOLDER)
	script.Exec(exec).Stdout()

	fmt.Println("You made it here!!!!!!!!!!!!!!!!")
}
