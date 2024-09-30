package main

import (
	"cmd/common/configuration"
	"cmd/common/exit"
	"cmd/common/flags"
	"cmd/common/flags/buildmode"
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

var buildMode = buildmode.PossibleBuildModes[0]

var isHelp = false

func usage() {
	flags.DisplayUsage("")
	fmt.Printf("\n")
	flags.DisplayOptionalFlags()

	buildmode.DisplayBuildMode(buildMode)

	fmt.Printf("\n")
	exit.DisplayExitCodes()
	exit.DisplayExitCode(exit.EXIT_SUCCESS)
	exit.DisplayExitCode(exit.EXIT_MISSING_ARGUMENT)
	exit.DisplayExitCode(exit.EXIT_CLI_PARSING_ERROR)
	fmt.Printf("\n")
	flags.DisplayExamples()
	fmt.Printf("  %s\n", filepath.Base(os.Args[0]))
	fmt.Printf("  %s -%s=%s\n", filepath.Base(os.Args[0]),
		buildmode.BUILD_MODE_LONG_FLAG, buildmode.PossibleBuildModes[1])
	fmt.Printf("\n")
}

func main() {
	buildmode.AddBuildModeAsFlag(&buildMode)

	flag.Usage = usage
	flag.Parse()

	var showHelpAndExit = false

	if !buildmode.IsValidBuildMode(buildMode) {
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

	configuration.DisplayConfiguration()
	buildmode.DisplayBuildModeConfiguration(buildMode)
	fmt.Printf("\n")

	// cmd/compile.elf "${BUILD_OPTIONS[@]}"
	// cmd/create-test-image.sh "${BUILD_OPTIONS[@]}"

}
