package main

import (
	"cmd/common/configuration"
	"cmd/common/exit"
	"cmd/common/flags"
	"cmd/common/flags/buildmode"
	"cmd/common/uefiimage"
	"cmd/compile/builder"
	"cmd/run-qemu/qemu"
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

var buildArgs = builder.RunBuildArgs
var qemuArgs = qemu.DefaultQemuArgs

var isHelp = false

func usage() {
	flags.DisplayUsage("")
	fmt.Printf("\n")
	flags.DisplayOptionalFlags()

	buildmode.DisplayBuildMode(buildArgs.BuildMode)

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
	buildmode.AddBuildModeAsFlag(&buildArgs.BuildMode)

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

	configuration.DisplayConfiguration()
	buildmode.DisplayBuildModeConfiguration(buildArgs.BuildMode)
	fmt.Printf("\n")

	var result = builder.Build(&buildArgs)
	if result != builder.Success {
		os.Exit(exit.EXIT_TARGET_ERROR)
	}

	uefiimage.CreateUefiImage(buildArgs.BuildMode)

	if buildArgs.BuildMode == string(buildmode.Debug) {
		qemuArgs.Debug = true
		qemuArgs.Verbose = true
	}
	qemu.Run(&qemuArgs)
}
