package projects

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/cmake"
	"fmt"
	"strings"

	"github.com/bitfield/script"
)

func findAndRunTests(args *BuildArgs, kernelBuildDirectory string) BuildResult {
	if len(args.SelectedTargets) > 0 {
		for _, target := range args.SelectedTargets {
			var findCommand = fmt.Sprintf("find %s -executable -type f -name \"%s\"", kernelBuildDirectory, target)
			var testFile, err = script.Exec(findCommand).String()
			if err != nil {
				fmt.Sprintf("%sFinding test targets to run errored!%s\n", common.RED, common.RESET)
				fmt.Println(findCommand)
				fmt.Println(err)
				return Failure
			}
			script.Exec(testFile).Stdout()
		}

		return Success
	}

	var findCommand = fmt.Sprintf("find %s -executable -type f -name \"*-tests*\" -exec {} \\;", kernelBuildDirectory)
	script.Exec(findCommand).Stdout()

	return Success
}

func displayProjectBuild(project string) {
	fmt.Printf("%sGoing to build %s project%s\n", common.CYAN, project, common.RESET)
}

func buildStandardProject(args *BuildArgs, codeFolder string) {
	displayProjectBuild(codeFolder)

	var buildDirectory = codeFolder + "/build"

	configureOptions := strings.Builder{}
	cmake.AddCommonConfigureOptions(&configureOptions, codeFolder, buildDirectory, args.CCompiler, args.Linker, args.BuildMode)

	argument.RunCommand(common.CMAKE_EXECUTABLE, configureOptions.String())

	buildOptions := strings.Builder{}
	cmake.AddCommonBuildOptions(&buildOptions, buildDirectory, args.Threads)

	argument.RunCommand(common.CMAKE_EXECUTABLE, buildOptions.String())
}

func buildKernel(args *BuildArgs, buildDirectory string) {
	displayProjectBuild(common.KERNEL_CODE_FOLDER)

	configureOptions := strings.Builder{}
	cmake.AddCommonConfigureOptions(&configureOptions, common.KERNEL_CODE_FOLDER, buildDirectory, args.CCompiler, args.Linker, args.BuildMode)
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D USE_AVX=%t", args.UseAVX))
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D USE_SSE=%t", args.UseSSE))
	argument.AddArgument(&configureOptions, fmt.Sprintf("-D UNIT_TEST_BUILD=%t", args.TestBuild))

	argument.RunCommand(common.CMAKE_EXECUTABLE, configureOptions.String())

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

	argument.RunCommand(common.CMAKE_EXECUTABLE, buildOptions.String())

	findOptions := strings.Builder{}
	argument.AddArgument(&findOptions, buildDirectory)
	argument.AddArgument(&findOptions, "-maxdepth 1")
	argument.AddArgument(&findOptions, "-name \"compile_commands.json\"")
	// NOTE: Using copy here because sym linking gave issues???
	argument.AddArgument(&findOptions, fmt.Sprintf("-exec cp -f {} %s \\;", common.KERNEL_CODE_FOLDER))

	argument.RunCommand("find", findOptions.String())
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

func Build(args *BuildArgs) BuildResult {
	// NOTE: Using waitgroups is currently slower than just running synchronously

	var kernelBuildDirectory = cmake.KernelBuildDirectory(args.TestBuild, args.CCompiler)
	buildKernel(args, kernelBuildDirectory)

	fmt.Println("Building Because of LSP purposes")
	buildStandardProject(args, common.INTEROPERATION_CODE_FOLDER)

	if args.TestBuild {
		if !args.RunTests {
			return Success
		}
		findAndRunTests(args, kernelBuildDirectory)
		return Success
	}

	buildStandardProject(args, common.UEFI_IMAGE_CREATOR_CODE_FOLDER)
	buildStandardProject(args, common.UEFI_CODE_FOLDER)
	return Success
}
