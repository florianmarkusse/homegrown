package main

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/cmake"
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

// TODO: Add flag to  redirect stderr on builds

const BUILD_MODE_LONG_FLAG = "build-mode"
const BUILD_MODE_SHORT_FLAG = "m"

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
const EXIT_TARGET_ERROR = 3

var possibleBuildModes = [...]string{"Release", "Debug", "Profiling", "Fuzzing"}
var buildMode = possibleBuildModes[0]

var cCompiler = "clang-19"

var linker = "ld"

var targets string
var usingTargets = false
var selectedTargets []string

var testBuild = false

var runTests = false

var threads = runtime.NumCPU()

var useAVX = true

var useSSE = true

var help = false

func displayProjectBuild(project string) {
	fmt.Printf("%sGoing to build %s project%s\n", common.CYAN, common.KERNEL_CODE_FOLDER, common.RESET)
}

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

func main() {
	flag.StringVar(&buildMode, BUILD_MODE_LONG_FLAG, buildMode, "")
	flag.StringVar(&buildMode, BUILD_MODE_SHORT_FLAG, buildMode, "")

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
		}
		os.Exit(EXIT_MISSING_ARGUMENT)
	}

	selectedTargets = strings.FieldsFunc(targets, func(r rune) bool {
		return r == ','
	})
	usingTargets = len(selectedTargets) > 0

	configuration.DisplayConfiguration()
	configuration.DisplayStringArgument(BUILD_MODE_LONG_FLAG, buildMode)
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

	// NOTE: Using waitgroups is currently slower than just running synchronously

	var kernelBuildDirectory = cmake.KernelBuildDirectory(testBuild, cCompiler)
	buildKernel(kernelBuildDirectory)

	fmt.Println("Building Because of LSP purposes")
	buildStandardProject(common.INTEROPERATION_CODE_FOLDER)

	if testBuild {
		findAndRunTests(kernelBuildDirectory)
	}

	buildStandardProject(common.UEFI_IMAGE_CREATOR_CODE_FOLDER)
	buildStandardProject(common.UEFI_CODE_FOLDER)
}

func findAndRunTests(kernelBuildDirectory string) {
	if !runTests {
		os.Exit(EXIT_SUCCESS)
	}

	if usingTargets {
		for _, target := range selectedTargets {
			var findCommand = fmt.Sprintf("find %s -executable -type f -name \"%s\"", kernelBuildDirectory, target)
			var testFile, err = script.Exec(findCommand).String()
			if err != nil {
				fmt.Sprintf("%sFinding test targets to run errored!%s\n", common.RED, common.RESET)
				fmt.Println(findCommand)
				fmt.Println(err)
				os.Exit(EXIT_TARGET_ERROR)
			}
			script.Exec(testFile).Stdout()
		}

		os.Exit(EXIT_SUCCESS)
	}

	var findCommand = fmt.Sprintf("find %s -executable -type f -name \"*-tests*\" -exec {} \\;", kernelBuildDirectory)
	script.Exec(findCommand).Stdout()

	os.Exit(EXIT_SUCCESS)
}

func usage() {
	flags.DisplayUsage("")
	fmt.Printf("\n")
	flags.DisplayOptionalFlags()

	// Not sure why go doesnt understand string lengths of this one, but whatever
	var buildModeDescription = fmt.Sprintf("Set the build mode (%s%s%s)        ", common.WHITE,
		arrayIntoPrintableString(possibleBuildModes[:]), common.RESET)
	flags.DisplayArgumentInput(BUILD_MODE_SHORT_FLAG, BUILD_MODE_LONG_FLAG, buildModeDescription, buildMode)

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
	flags.DisplayExitCode(EXIT_TARGET_ERROR, "Targets to build error")
	fmt.Printf("\n")
	flags.DisplayExamples()
	fmt.Printf("  %s\n", filepath.Base(os.Args[0]))
	fmt.Printf("  %s -%s=%s --%s text,log --%s -%s\n", filepath.Base(os.Args[0]),
		BUILD_MODE_LONG_FLAG, possibleBuildModes[1], SELECT_TARGETS_LONG_FLAG, TEST_BUILD_LONG_FLAG, RUN_TESTS_SHORT_FLAG)
	fmt.Printf("\n")
}

func buildKernel(buildDirectory string) {
	displayProjectBuild(common.KERNEL_CODE_FOLDER)

	configureOptions := strings.Builder{}
	cmake.AddCommonConfigureOptions(&configureOptions, common.KERNEL_CODE_FOLDER, buildDirectory, cCompiler, linker, buildMode)
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D USE_AVX=%t", useAVX))
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D USE_SSE=%t", useSSE))
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D UNIT_TEST_BUILD=%t", testBuild))

	argument.RunCommand(common.CMAKE_EXECUTABLE, configureOptions.String())

	buildOptions := strings.Builder{}
	cmake.AddCommonBuildOptions(&buildOptions, buildDirectory, threads)

	if usingTargets {
		targetsString := strings.Builder{}
		for _, target := range selectedTargets {
			targetsString.WriteString(target)
			targetsString.WriteString(" ")
		}
		argument.AddArgument(&buildOptions, fmt.Sprintf("--target %s", targetsString.String()))
	}

	argument.RunCommand(common.CMAKE_EXECUTABLE, buildOptions.String())

	findOptions := strings.Builder{}
	argument.AddArgument(&findOptions, buildDirectory)
	argument.AddArgument(&findOptions, "-maxdepth 1")
	argument.AddArgument(&findOptions, "-name \"compile_commands.json\"")
	// NOTE: Using copy here because sym linking gave issues???
	argument.AddArgument(&findOptions, fmt.Sprintf("-exec cp -f {} %s \\;", common.KERNEL_CODE_FOLDER))

	argument.RunCommand("find", findOptions.String())
}

func buildStandardProject(codeFolder string) {
	displayProjectBuild(codeFolder)

	var buildDirectory = codeFolder + "/build"

	configureOptions := strings.Builder{}
	cmake.AddCommonConfigureOptions(&configureOptions, codeFolder, buildDirectory, cCompiler, linker, buildMode)

	argument.RunCommand(common.CMAKE_EXECUTABLE, configureOptions.String())

	buildOptions := strings.Builder{}
	cmake.AddCommonBuildOptions(&buildOptions, buildDirectory, threads)

	argument.RunCommand(common.CMAKE_EXECUTABLE, buildOptions.String())
}
