package cmake

import (
	"bufio"
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/exit"
	"cmd/common/flags/environment"
	"cmd/common/project"
	"fmt"
	"os"
	"strings"
)

const EXECUTABLE = "cmake"

func AddDefaultConfigureOptions(options *strings.Builder, codeFolder string, buildDirectory string, cCompiler string, linker string, buildMode string, env string, buildTests bool, projectTargetsFile string, architecture string) {
	argument.AddArgument(options, fmt.Sprintf("-S %s", codeFolder))
	argument.AddArgument(options, fmt.Sprintf("-B %s", buildDirectory))
	argument.AddArgument(options, fmt.Sprintf("-D CMAKE_C_COMPILER=%s", cCompiler))
	argument.AddArgument(options, fmt.Sprintf("-D CMAKE_LINKER=%s", linker))
	argument.AddArgument(options, fmt.Sprintf("-D CMAKE_BUILD_TYPE=%s", buildMode))
	argument.AddArgument(options, fmt.Sprintf("-D ENVIRONMENT=%s", env))
	argument.AddArgument(options, fmt.Sprintf("-D ARCHITECTURE=%s", architecture))
	argument.AddArgument(options, fmt.Sprintf("-D BUILD_OUTPUT_PATH=%s", project.BuildOutputPath(cCompiler, linker, env, buildMode)))
	argument.AddArgument(options, fmt.Sprintf("-D REPO_ROOT=%s", common.REPO_ROOT))
	argument.AddArgument(options, fmt.Sprintf("-D REPO_DEPENDENCIES=%s", common.REPO_DEPENDENCIES))
	argument.AddArgument(options, fmt.Sprintf("-D REPO_PROJECTS=%s", common.REPO_PROJECTS))
	argument.AddArgument(options, fmt.Sprintf("-D PROJECT_TARGETS_FILE=%s", projectTargetsFile))

	argument.AddArgument(options, fmt.Sprintf("--graphviz=%s/output.dot", codeFolder))
	argument.AddArgument(options, fmt.Sprintf("-D UNIT_TEST_BUILD=%t", buildTests))

	var iwyuString = strings.Builder{}
	iwyuString.WriteString("-D CMAKE_C_INCLUDE_WHAT_YOU_USE=\"include-what-you-use;-w;-Xiwyu;")
	if env == string(environment.Freestanding) {
		iwyuString.WriteString("--no_default_mappings")
	}
	iwyuString.WriteString("\"")
	argument.AddArgument(options, iwyuString.String())
}

func AddDefaultBuildOptions(options *strings.Builder, buildDirectory string, projectTargetsFile string, threads int, targets []string, verbose bool) bool {
	argument.AddArgument(options, fmt.Sprintf("--build %s", buildDirectory))
	argument.AddArgument(options, fmt.Sprintf("--parallel %d", threads))
	if verbose {
		argument.AddArgument(options, "-v")
	}

	targetsString := strings.Builder{}
	if len(targets) > 0 {
		for _, target := range targets {
			targetsString.WriteString(target)
			targetsString.WriteString(" ")
		}
	} else {
		file, err := os.Open(projectTargetsFile)
		if err != nil {
			os.Exit(exit.EXIT_TARGET_ERROR)
		}
		defer file.Close()

		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			var target = scanner.Text()
			targetsString.WriteString(target)
			targetsString.WriteString(" ")
		}

		if err := scanner.Err(); err != nil {
			os.Exit(exit.EXIT_TARGET_ERROR)
		}

		if targetsString.Len() == 0 {
			return false
		}

	}

	if targetsString.Len() > 0 {
		argument.AddArgument(options, fmt.Sprintf("--target %s", targetsString.String()))
	}

	return true
}
