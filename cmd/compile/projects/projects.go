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

func findAndRunTests(args *BuildArgs, codeDirectory string) BuildResult {
	var buildDirectory = cmake.BuildDirectoryRoot(codeDirectory, args.TestBuild, args.CCompiler)
	if len(args.SelectedTargets) > 0 {
		for _, target := range args.SelectedTargets {
			var findCommand = fmt.Sprintf("find %s -executable -type f -name \"%s\" -exec {} \\;", buildDirectory, target)
			argument.ExecCommand(findCommand)
		}

		return Success
	}

	var findCommand = fmt.Sprintf("find %s -executable -type f -name \"*-tests*\" -exec {} \\;", buildDirectory)
	argument.ExecCommand(findCommand)

	return Success
}

func displayProjectBuild(project string) {
	fmt.Printf("%sGoing to build %s project%s\n", common.CYAN, project, common.RESET)
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

func buildStandardProject(args *BuildArgs, project *common.ProjectStructure) {
	displayProjectBuild(project.CodeFolder)

	var buildDirectory = cmake.BuildDirectoryRoot(project.CodeFolder, args.TestBuild, args.CCompiler)

	var errorWriters []io.Writer = populateErrorWriter(args.ErrorsToFile, project.CodeFolder)

	configureOptions := strings.Builder{}
	cmake.AddCommonConfigureOptions(&configureOptions, project.CodeFolder, buildDirectory, args.CCompiler, args.Linker, args.BuildMode, project.IsFreeStanding)

	argument.ExecCommandWriteError(fmt.Sprintf("%s %s", common.CMAKE_EXECUTABLE, configureOptions.String()), errorWriters...)

	buildOptions := strings.Builder{}
	cmake.AddCommonBuildOptions(&buildOptions, buildDirectory, args.Threads)

	argument.ExecCommandWriteError(fmt.Sprintf("%s %s", common.CMAKE_EXECUTABLE, buildOptions.String()), errorWriters...)

	copyCompileCommands(buildDirectory, project.CodeFolder)
}

func buildKernel(args *BuildArgs, project *common.ProjectStructure) {
	displayProjectBuild(project.CodeFolder)

	var buildDirectory = cmake.BuildDirectoryRoot(project.CodeFolder, args.TestBuild, args.CCompiler)

	var errorWriters []io.Writer = populateErrorWriter(args.ErrorsToFile, project.CodeFolder)

	configureOptions := strings.Builder{}
	cmake.AddCommonConfigureOptions(&configureOptions, project.CodeFolder, buildDirectory, args.CCompiler, args.Linker, args.BuildMode, project.IsFreeStanding)
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D USE_AVX=%t", args.UseAVX))
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D USE_SSE=%t", args.UseSSE))
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D UNIT_TEST_BUILD=%t", args.TestBuild))

	argument.ExecCommandWriteError(fmt.Sprintf("%s %s", common.CMAKE_EXECUTABLE, configureOptions.String()), errorWriters...)

	buildOptions := strings.Builder{}
	cmake.AddCommonBuildOptions(&buildOptions, buildDirectory, args.Threads)

	if len(args.SelectedTargets) > 0 {
		targetsString := strings.Builder{}
		for _, target := range args.SelectedTargets {
			targetsString.WriteString(target)
			targetsString.WriteString(" ")
		}
		argument.AddArgument(&buildOptions, fmt.Sprintf("--target %s", targetsString.String()))
	}

	argument.ExecCommandWriteError(fmt.Sprintf("%s %s", common.CMAKE_EXECUTABLE, buildOptions.String()), errorWriters...)

	copyCompileCommands(buildDirectory, project.CodeFolder)
}

type BuildArgs struct {
	CCompiler       string
	Linker          string
	BuildMode       string
	ErrorsToFile    bool
	Threads         int
	SelectedTargets []string
	Projects        []string
	TestBuild       bool
	RunTests        bool
	UseAVX          bool
	UseSSE          bool
}

type BuildResult uint8

const (
	Success BuildResult = iota
	Failure
)

var DefaultBuildArgs = BuildArgs{
	CCompiler:       "clang-19",
	Linker:          "ld",
	BuildMode:       buildmode.DefaultBuildMode(),
	ErrorsToFile:    false,
	Threads:         runtime.NumCPU(),
	SelectedTargets: []string{},
	Projects:        []string{},
	TestBuild:       false,
	RunTests:        false,
	UseAVX:          true,
	UseSSE:          true,
}

func Build(args *BuildArgs) BuildResult {
	// NOTE: Using waitgroups is currently slower than just running synchronously

	if len(args.Projects) > 0 {
	}

	buildKernel(args, &common.PROJECTS[common.KERNEL])

	fmt.Println("Building Because of LSP purposes")
	buildStandardProject(args, &common.PROJECTS[common.INTEROPERATION])

	if args.TestBuild {
		if !args.RunTests {
			return Success
		}
		findAndRunTests(args, common.PROJECTS[common.KERNEL].CodeFolder)
		return Success
	}

	buildStandardProject(args, &common.PROJECTS[common.UEFI_IMAGE_CREATOR])
	buildStandardProject(args, &common.PROJECTS[common.UEFI])
	buildStandardProject(args, &common.PROJECTS[common.IMAGE_BUILDER])
	return Success
}
