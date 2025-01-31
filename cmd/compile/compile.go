package main

import (
	"cmd/common/configuration"
	"cmd/common/converter"
	"cmd/common/exit"
	"cmd/common/flags"
	"cmd/common/flags/architecture"
	"cmd/common/flags/buildmode"
	"cmd/common/flags/environment"
	"cmd/common/flags/help"
	"cmd/common/project"
	"cmd/compile/builder"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

const ERRORS_TO_FILE_LONG_FLAG = "errors-to-file"
const ERRORS_TO_FILE_SHORT_FLAG = "e"

const SELECT_TARGETS_LONG_FLAG = "targets"
const SELECT_TARGETS_SHORT_FLAG = "s"
const DEFAULT_TARGETS = "All targets starting with the project's name."

const BUILD_TESTS_LONG_FLAG = "test-build"
const BUILD_TESTS_SHORT_FLAG = "t"

const RUN_TESTS_LONG_FLAG = "run-tests"
const RUN_TESTS_SHORT_FLAG = "r"

const VERBOSE_LONG_FLAG = "verbose"
const VERBOSE_SHORT_FLAG = "v"

const THREADS_LONG_FLAG = "threads"

var buildArgs = builder.DefaultBuildArgs

var projectsToBuild string
var targetsToBuild string

var isHelp = false

func main() {
	buildmode.AddBuildModeAsFlag(&buildArgs.BuildMode)
	architecture.AddArchitectureAsFlag(&buildArgs.Architecture)
	environment.AddEnvironmentAsFlag(&buildArgs.Environment)
	project.AddProjectAsFlag(&projectsToBuild)
	help.AddHelpAsFlag(&isHelp)

	flag.StringVar(&targetsToBuild, SELECT_TARGETS_LONG_FLAG, "", "")
	flag.StringVar(&targetsToBuild, SELECT_TARGETS_SHORT_FLAG, "", "")

	flag.BoolVar(&buildArgs.ErrorsToFile, ERRORS_TO_FILE_LONG_FLAG, buildArgs.ErrorsToFile, "")
	flag.BoolVar(&buildArgs.ErrorsToFile, ERRORS_TO_FILE_SHORT_FLAG, buildArgs.ErrorsToFile, "")

	flag.BoolVar(&buildArgs.BuildTests, BUILD_TESTS_LONG_FLAG, buildArgs.BuildTests, "")
	flag.BoolVar(&buildArgs.BuildTests, BUILD_TESTS_SHORT_FLAG, buildArgs.BuildTests, "")

	flag.BoolVar(&buildArgs.RunTests, RUN_TESTS_LONG_FLAG, buildArgs.RunTests, "")
	flag.BoolVar(&buildArgs.RunTests, RUN_TESTS_SHORT_FLAG, buildArgs.RunTests, "")

	flag.BoolVar(&buildArgs.Verbose, VERBOSE_LONG_FLAG, buildArgs.Verbose, "")
	flag.BoolVar(&buildArgs.Verbose, VERBOSE_SHORT_FLAG, buildArgs.Verbose, "")

	flag.IntVar(&buildArgs.Threads, THREADS_LONG_FLAG, buildArgs.Threads, "")

	flag.Usage = usage
	flag.Parse()

	var showHelpAndExit = false

	if !buildmode.IsValidBuildMode(buildArgs.BuildMode) {
		showHelpAndExit = true
	}

	if !environment.IsValidEnvironment(buildArgs.Environment) && buildArgs.Environment != "" {
		showHelpAndExit = true
	}

	if !architecture.IsValidArchitecture(buildArgs.Architecture) {
		showHelpAndExit = true
	}

	if !project.ValidateAndConvertProjects(projectsToBuild, &buildArgs.SelectedProjects) {
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

	buildArgs.SelectedTargets = strings.FieldsFunc(targetsToBuild, func(r rune) bool {
		return r == ','
	})

	configuration.DisplayConfiguration()
	buildmode.DisplayBuildModeConfiguration(buildArgs.BuildMode)
	architecture.DisplayArchitectureConfiguration(buildArgs.Architecture)
	environment.DisplayEnvironmentConfiguration(buildArgs.Environment)
	project.DisplayProjectConfiguration(buildArgs.SelectedProjects)

	var targetsConfiguration string
	if len(buildArgs.SelectedTargets) > 0 {
		targetsConfiguration = converter.ArrayIntoPrintableString(buildArgs.SelectedTargets[:])
	} else {
		targetsConfiguration = DEFAULT_TARGETS
	}
	configuration.DisplayStringArgument(SELECT_TARGETS_LONG_FLAG, targetsConfiguration)

	configuration.DisplayBoolArgument(BUILD_TESTS_LONG_FLAG, buildArgs.BuildTests)
	configuration.DisplayBoolArgument(RUN_TESTS_LONG_FLAG, buildArgs.RunTests)
	configuration.DisplayBoolArgument(VERBOSE_LONG_FLAG, buildArgs.Verbose)
	configuration.DisplayIntArgument(THREADS_LONG_FLAG, buildArgs.Threads)

	fmt.Printf("\n")

	var result = builder.Build(&buildArgs)

	switch result {
	case builder.Success:
		{
			os.Exit(exit.EXIT_SUCCESS)
		}
	case builder.Failure:
		{
			os.Exit(exit.EXIT_TARGET_ERROR)
		}
	}
}

func usage() {
	flags.DisplayUsage("")
	fmt.Printf("\n")
	flags.DisplayOptionalFlags()

	buildmode.DisplayBuildMode()
	environment.DisplayEnvironment()
	architecture.DisplayArchitecture()
	project.DisplayProject()

	flags.DisplayArgumentInput(ERRORS_TO_FILE_SHORT_FLAG, ERRORS_TO_FILE_LONG_FLAG, "Save errors to file", fmt.Sprint(buildArgs.ErrorsToFile))

	flags.DisplayArgumentInput(SELECT_TARGETS_SHORT_FLAG, SELECT_TARGETS_LONG_FLAG, "Select specific target(s, comma-separated) to be built", DEFAULT_TARGETS)

	flags.DisplayArgumentInput(BUILD_TESTS_SHORT_FLAG, BUILD_TESTS_LONG_FLAG, "Build for tests", fmt.Sprint(buildArgs.BuildTests))

	flags.DisplayArgumentInput(RUN_TESTS_SHORT_FLAG, RUN_TESTS_LONG_FLAG, "Run tests", fmt.Sprint(buildArgs.RunTests))

	flags.DisplayArgumentInput(VERBOSE_SHORT_FLAG, VERBOSE_LONG_FLAG, "Turn on verbose logging", fmt.Sprint(buildArgs.Verbose))

	flags.DisplayLongFlagArgumentInput(THREADS_LONG_FLAG, "Set the number of threads to use for compiling", fmt.Sprint(buildArgs.Threads))

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
	fmt.Printf("  %s --%s=%s --%s %s,%s --%s -%s\n", filepath.Base(os.Args[0]),
		buildmode.BUILD_MODE_LONG_FLAG, buildmode.PossibleBuildModes[1], project.PROJECTS_LONG_FLAG, project.KERNEL, project.OS_LOADER, BUILD_TESTS_LONG_FLAG, RUN_TESTS_SHORT_FLAG)
	fmt.Printf("\n")
}
