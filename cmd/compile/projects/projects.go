package projects

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/cmake"
	"cmd/common/flags/buildmode"
	"fmt"
	"io"
	"log"
	"os"
	"runtime"
	"strings"
)

func findAndRunTests(selectedTargets []string, project *cmake.ProjectStructure, buildMode string) BuildResult {
	var buildDirectory = cmake.BuildDirectoryRoot(project, buildMode)
	if len(selectedTargets) > 0 {
		for _, target := range selectedTargets {
			var findCommand = fmt.Sprintf("find %s -executable -type f -name \"%s\" -exec {} \\;", buildDirectory, target)
			argument.ExecCommand(findCommand)
		}

		return Success
	}

	var findCommand = fmt.Sprintf("find %s -executable -type f -name \"*-tests*\" -exec {} \\;", buildDirectory)
	argument.ExecCommand(findCommand)

	return Success
}

func copyCompileCommands(buildDirectory string, codeDirectory string) {
	findOptions := strings.Builder{}
	argument.AddArgument(&findOptions, buildDirectory)
	argument.AddArgument(&findOptions, "-maxdepth 1")
	argument.AddArgument(&findOptions, "-name \"compile_commands.json\"")
	// NOTE: Using copy here because sym linking gave issues???
	argument.AddArgument(&findOptions, fmt.Sprintf("-exec cp -f {} %s \\;", codeDirectory))

	argument.ExecCommand(fmt.Sprintf("find %s", findOptions.String()))
}

func populateErrorWriter(errorsToFile bool, codeDirectory string) []io.Writer {
	if !errorsToFile {
		return []io.Writer{}
	}

	var errorFile = fmt.Sprintf("%s/stderr.txt", codeDirectory)
	file, err := os.Create(errorFile)
	if err != nil {
		log.Fatalf("Failed to create file to redirect errors to")
	}

	var result = []io.Writer{file}
	return result
}

func buildProject(args *BuildArgs, project *cmake.ProjectStructure) {
	var buildDirectory = cmake.BuildDirectoryRoot(project, args.BuildMode)

	var errorWriters []io.Writer = populateErrorWriter(args.ErrorsToFile, project.CodeFolder)

	configureOptions := strings.Builder{}
	var environment string = string(project.Environment)
	if args.Environment != "" {
		environment = string(args.Environment)
	}

	cmake.AddDefaultConfigureOptions(&configureOptions, project.CodeFolder, buildDirectory, project.CCompiler, project.Linker, args.BuildMode, environment, args.BuildTests)
	argument.ExecCommandWriteError(fmt.Sprintf("%s %s", cmake.EXECUTABLE, configureOptions.String()), errorWriters...)

	buildOptions := strings.Builder{}
	cmake.AddDefaultBuildOptions(&buildOptions, buildDirectory, args.Threads, args.SelectedTargets)
	argument.ExecCommandWriteError(fmt.Sprintf("%s %s", cmake.EXECUTABLE, buildOptions.String()), errorWriters...)

	copyCompileCommands(buildDirectory, project.CodeFolder)
}

type BuildArgs struct {
	BuildMode        string
	Environment      string
	ErrorsToFile     bool
	Threads          int
	SelectedTargets  []string
	SelectedProjects []string
	BuildTests       bool
	RunTests         bool
}

type BuildResult uint8

const (
	Success BuildResult = iota
	Failure
)

var DefaultBuildArgs = BuildArgs{
	BuildMode:        buildmode.DefaultBuildMode(),
	Environment:      "",
	ErrorsToFile:     false,
	Threads:          runtime.NumCPU(),
	SelectedTargets:  []string{},
	SelectedProjects: []string{},
	BuildTests:       false,
	RunTests:         false,
}

func getAllProjects(selectedProjects []string) map[string]*cmake.ProjectStructure {
	if len(selectedProjects) == 0 {
		return cmake.PROJECT_STRUCTURES
	}

	result := make(map[string]*cmake.ProjectStructure)

	for _, name := range selectedProjects {
		var project = cmake.PROJECT_STRUCTURES[name]
		result[name] = project
	}

	return result
}

func Build(args *BuildArgs) BuildResult {
	// NOTE: Using waitgroups is currently slower than just running synchronously

	var projectsToBuild = getAllProjects(args.SelectedProjects)
	for name, project := range projectsToBuild {
		fmt.Printf("Building %s%s%s\n", common.CYAN, name, common.RESET)
		buildProject(args, project)
	}

	if args.BuildTests {
		if !args.RunTests {
			return Success
		}
		for name, project := range projectsToBuild {
			fmt.Printf("Testing %s%s%s\n", common.CYAN, name, common.RESET)
			findAndRunTests(args.SelectedTargets, project, args.BuildMode)
		}
	}

	return Success
}
