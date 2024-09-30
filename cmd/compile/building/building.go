package building

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/cmake"
	"fmt"
	"os"
	"strings"

	"github.com/bitfield/script"
)

func findAndRunTests(kernelBuildDirectory string) {
	if !runTests {
		os.Exit(EXIT_SUCCESS)
	}

	if len(selectedTargets) > 0 {
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

func displayProjectBuild(project string) {
	fmt.Printf("%sGoing to build %s project%s\n", common.CYAN, project, common.RESET)
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

func Build() {
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