package main

import (
	"cmd/common/configuration"
	"cmd/common/converter"
	"cmd/common/exit"
	"cmd/common/flags"
	"cmd/common/flags/buildmode"
	"cmd/common/flags/help"
	"cmd/compile/projects"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// TODO: Add flag to  redirect stderr on builds

const C_COMPILER_LONG_FLAG = "c-compiler"
const C_COMPILER_SHORT_FLAG = "c"

const LINKER_LONG_FLAG = "linker"
const LINKER_SHORT_FLAG = "l"

const ERRORS_TO_FILE_LONG_FLAG = "errors-to-file"
const ERRORS_TO_FILE_SHORT_FLAG = "e"

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

var buildArgs = projects.DefaultBuildArgs

var targets string

var isHelp = false

func main() {
	buildmode.AddBuildModeAsFlag(&buildArgs.BuildMode)

	flag.StringVar(&buildArgs.CCompiler, C_COMPILER_LONG_FLAG, buildArgs.CCompiler, "")
	flag.StringVar(&buildArgs.CCompiler, C_COMPILER_SHORT_FLAG, buildArgs.CCompiler, "")

	flag.StringVar(&buildArgs.Linker, LINKER_LONG_FLAG, buildArgs.Linker, "")
	flag.StringVar(&buildArgs.Linker, LINKER_SHORT_FLAG, buildArgs.Linker, "")

	flag.StringVar(&targets, SELECT_TARGETS_LONG_FLAG, "", "")
	flag.StringVar(&targets, SELECT_TARGETS_SHORT_FLAG, "", "")

	flag.BoolVar(&buildArgs.ErrorsToFile, ERRORS_TO_FILE_LONG_FLAG, buildArgs.ErrorsToFile, "")
	flag.BoolVar(&buildArgs.ErrorsToFile, ERRORS_TO_FILE_SHORT_FLAG, buildArgs.ErrorsToFile, "")

	flag.BoolVar(&buildArgs.TestBuild, TEST_BUILD_LONG_FLAG, buildArgs.TestBuild, "")
	flag.BoolVar(&buildArgs.TestBuild, TEST_BUILD_SHORT_FLAG, buildArgs.TestBuild, "")

	flag.BoolVar(&buildArgs.RunTests, RUN_TESTS_LONG_FLAG, buildArgs.RunTests, "")
	flag.BoolVar(&buildArgs.RunTests, RUN_TESTS_SHORT_FLAG, buildArgs.RunTests, "")

	flag.IntVar(&buildArgs.Threads, THREADS_LONG_FLAG, buildArgs.Threads, "")

	flag.BoolVar(&buildArgs.UseAVX, AVX_LONG_FLAG, buildArgs.UseAVX, "")

	flag.BoolVar(&buildArgs.UseSSE, SSE_LONG_FLAG, buildArgs.UseSSE, "")

	help.AddHelpAsFlag(&isHelp)

	flag.Usage = usage
	flag.Parse()

	var showHelpAndExit = false

	if !buildmode.IsValidBuildMode(buildArgs.BuildMode) {
		showHelpAndExit = true
	}

	if isHelp {
		showHelpAndExit = true
	}

	if showHelpAndExit {
		usage()
		if isHelp {
			os.Exit(exit.EXIT_SUCCESS)
		}
		os.Exit(exit.EXIT_MISSING_ARGUMENT)
	}

	buildArgs.SelectedTargets = strings.FieldsFunc(targets, func(r rune) bool {
		return r == ','
	})

	configuration.DisplayConfiguration()
	buildmode.DisplayBuildModeConfiguration(buildArgs.BuildMode)
	configuration.DisplayStringArgument(C_COMPILER_LONG_FLAG, buildArgs.CCompiler)
	configuration.DisplayStringArgument(LINKER_LONG_FLAG, buildArgs.Linker)

	var targetsConfiguration string
	if len(buildArgs.SelectedTargets) > 0 {
		targetsConfiguration = converter.ArrayIntoPrintableString(buildArgs.SelectedTargets[:])
	} else {
		targetsConfiguration = DEFAULT_TARGETS
	}
	configuration.DisplayStringArgument(SELECT_TARGETS_LONG_FLAG, targetsConfiguration)

	configuration.DisplayBoolArgument(TEST_BUILD_LONG_FLAG, buildArgs.TestBuild)
	configuration.DisplayBoolArgument(RUN_TESTS_LONG_FLAG, buildArgs.RunTests)
	configuration.DisplayIntArgument(THREADS_LONG_FLAG, buildArgs.Threads)
	configuration.DisplayBoolArgument(AVX_LONG_FLAG, buildArgs.UseAVX)
	configuration.DisplayBoolArgument(SSE_LONG_FLAG, buildArgs.UseSSE)
	fmt.Printf("\n")

	var result = projects.Build(&buildArgs)

	switch result {
	case projects.Success:
		{
			os.Exit(exit.EXIT_SUCCESS)
		}
	case projects.Failure:
		{
			os.Exit(exit.EXIT_TARGET_ERROR)
		}
	}
}

func usage() {
	flags.DisplayUsage("")
	fmt.Printf("\n")
	flags.DisplayOptionalFlags()

	buildmode.DisplayBuildMode(buildArgs.BuildMode)

	flags.DisplayArgumentInput(C_COMPILER_SHORT_FLAG, C_COMPILER_LONG_FLAG, "Set the c-compiler", fmt.Sprint(buildArgs.CCompiler))

	flags.DisplayArgumentInput(LINKER_SHORT_FLAG, LINKER_LONG_FLAG, "Set the linker", fmt.Sprint(buildArgs.Linker))
	flags.DisplayArgumentInput(ERRORS_TO_FILE_SHORT_FLAG, ERRORS_TO_FILE_LONG_FLAG, "Save errors to file", fmt.Sprint(buildArgs.ErrorsToFile))

	flags.DisplayArgumentInput(SELECT_TARGETS_SHORT_FLAG, SELECT_TARGETS_LONG_FLAG, "Select specific target(s, comma-separated) to be built", DEFAULT_TARGETS)

	flags.DisplayArgumentInput(TEST_BUILD_SHORT_FLAG, TEST_BUILD_LONG_FLAG, "Build for tests", fmt.Sprint(buildArgs.TestBuild))

	flags.DisplayArgumentInput(RUN_TESTS_SHORT_FLAG, RUN_TESTS_LONG_FLAG, "Run tests", fmt.Sprint(buildArgs.RunTests))

	flags.DisplayLongFlagArgumentInput(THREADS_LONG_FLAG, "Set the number of threads to use for compiling", fmt.Sprint(buildArgs.Threads))

	flags.DisplayLongFlagArgumentInput(AVX_LONG_FLAG, "Set SSE", fmt.Sprint(buildArgs.UseSSE))

	flags.DisplayLongFlagArgumentInput(SSE_LONG_FLAG, "Set AVX", fmt.Sprint(buildArgs.UseAVX))

	help.DisplayHelp()
	fmt.Printf("\n")
	exit.DisplayExitCodes()
	exit.DisplayExitCode(exit.EXIT_SUCCESS)
	exit.DisplayExitCode(exit.EXIT_MISSING_ARGUMENT)
	exit.DisplayExitCode(exit.EXIT_CLI_PARSING_ERROR)
	exit.DisplayExitCode(exit.EXIT_TARGET_ERROR)
	fmt.Printf("\n")
	flags.DisplayExamples()
	fmt.Printf("  %s\n", filepath.Base(os.Args[0]))
	fmt.Printf("  %s -%s=%s --%s text,log --%s -%s\n", filepath.Base(os.Args[0]),
		buildmode.BUILD_MODE_LONG_FLAG, buildmode.PossibleBuildModes[1], SELECT_TARGETS_LONG_FLAG, TEST_BUILD_LONG_FLAG, RUN_TESTS_SHORT_FLAG)
	fmt.Printf("\n")
}
