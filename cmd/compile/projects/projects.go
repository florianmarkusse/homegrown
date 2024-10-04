package projects

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/cmake"
	"cmd/common/flags/buildmode"
	"fmt"
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

func buildStandardProject(args *BuildArgs, codeFolder string) {
	displayProjectBuild(codeFolder)

	var buildDirectory = cmake.BuildDirectoryRoot(codeFolder, args.TestBuild, args.CCompiler)

	configureOptions := strings.Builder{}
	cmake.AddCommonConfigureOptions(&configureOptions, codeFolder, buildDirectory, args.CCompiler, args.Linker, args.BuildMode)

	argument.ExecCommand(fmt.Sprintf("%s %s", common.CMAKE_EXECUTABLE, configureOptions.String()))

	buildOptions := strings.Builder{}
	cmake.AddCommonBuildOptions(&buildOptions, buildDirectory, args.Threads)

	argument.ExecCommand(fmt.Sprintf("%s %s", common.CMAKE_EXECUTABLE, buildOptions.String()))
}

func buildKernel(args *BuildArgs) {
	displayProjectBuild(common.KERNEL_CODE_FOLDER)

	var buildDirectory = cmake.BuildDirectoryRoot(common.KERNEL_CODE_FOLDER, args.TestBuild, args.CCompiler)

	configureOptions := strings.Builder{}
	cmake.AddCommonConfigureOptions(&configureOptions, common.KERNEL_CODE_FOLDER, buildDirectory, args.CCompiler, args.Linker, args.BuildMode)
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D USE_AVX=%t", args.UseAVX))
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D USE_SSE=%t", args.UseSSE))
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D UNIT_TEST_BUILD=%t", args.TestBuild))

	argument.ExecCommand(fmt.Sprintf("%s %s", common.CMAKE_EXECUTABLE, configureOptions.String()))

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

	argument.ExecCommand(fmt.Sprintf("%s %s", common.CMAKE_EXECUTABLE, buildOptions.String()))

	findOptions := strings.Builder{}
	argument.AddArgument(&findOptions, buildDirectory)
	argument.AddArgument(&findOptions, "-maxdepth 1")
	argument.AddArgument(&findOptions, "-name \"compile_commands.json\"")
	// NOTE: Using copy here because sym linking gave issues???
	argument.AddArgument(&findOptions, fmt.Sprintf("-exec cp -f {} %s \\;", common.KERNEL_CODE_FOLDER))

	argument.ExecCommand(fmt.Sprintf("find %s", findOptions.String()))
}

type BuildArgs struct {
	CCompiler       string
	Linker          string
	BuildMode       string
	Threads         int
	SelectedTargets []string
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
	Threads:         runtime.NumCPU(),
	SelectedTargets: []string{},
	TestBuild:       false,
	RunTests:        false,
	UseAVX:          true,
	UseSSE:          true,
}

func Build(args *BuildArgs) BuildResult {
	// NOTE: Using waitgroups is currently slower than just running synchronously
	buildKernel(args)

	fmt.Println("Building Because of LSP purposes")
	buildStandardProject(args, common.INTEROPERATION_CODE_FOLDER)

	if args.TestBuild {
		if !args.RunTests {
			return Success
		}
		findAndRunTests(args, common.KERNEL_CODE_FOLDER)
		return Success
	}

	buildStandardProject(args, common.UEFI_IMAGE_CREATOR_CODE_FOLDER)
	buildStandardProject(args, common.UEFI_CODE_FOLDER)
	return Success
}
