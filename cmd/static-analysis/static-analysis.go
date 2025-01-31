package main

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/configuration"
	"cmd/common/exit"
	"cmd/common/flags"
	"cmd/common/flags/help"
	"cmd/common/project"
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

var selectedProjects = []string{}
var projectsToBuild string

var isHelp = false

func usage() {
	flags.DisplayUsage("")
	fmt.Printf("\n")
	flags.DisplayOptionalFlags()
	project.DisplayProject()
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
	fmt.Printf("  %s --%s %s,%s \n", filepath.Base(os.Args[0]), project.PROJECTS_LONG_FLAG, project.KERNEL, project.OS_LOADER)
	fmt.Printf("\n")
}

func analyzeProject(proj *project.ProjectStructure) {
	var findCommand = fmt.Sprintf("clang-tidy -fix -fix-errors $(find %s -type d \\( -path %s/build \\) -prune -o -type f -name \"*.[ch]\" -print) -p %s", proj.CodeFolder, proj.CodeFolder, proj.CodeFolder)
	argument.ExecCommand(findCommand)
}

func main() {
	project.AddProjectAsFlag(&projectsToBuild)
	help.AddHelpAsFlag(&isHelp)

	flag.Usage = usage
	flag.Parse()

	var showHelpAndExit = false

	if !project.ValidateAndConvertProjects(projectsToBuild, &selectedProjects) {
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
	project.DisplayProjectConfiguration(selectedProjects)

	var projectsToBuild = project.GetAllProjects(selectedProjects)
	for name, proj := range projectsToBuild {
		fmt.Printf("Analyzing %s%s%s\n", common.CYAN, name, common.RESET)
		analyzeProject(proj)
	}
}
