package cmake

import (
	"cmd/common/argument"
	"fmt"
	"strings"
)

func BuildDirectoryRoot(codeDirectory string, testBuild bool, cCompiler string) string {
	buildDirectory := strings.Builder{}
	buildDirectory.WriteString(fmt.Sprintf("%s/", codeDirectory))
	buildDirectory.WriteString("build/")
	if testBuild {
		buildDirectory.WriteString("test/")
	} else {
		buildDirectory.WriteString("prod/")
	}
	buildDirectory.WriteString(fmt.Sprintf("%s/", cCompiler))

	return buildDirectory.String()
}

func AddCommonConfigureOptions(options *strings.Builder, codeDirectory string, buildDirectory string, cCompiler string, linker string, buildMode string) {
	argument.AddArgument(options, fmt.Sprintf("-S %s", codeDirectory))
	argument.AddArgument(options, fmt.Sprintf("-B %s", buildDirectory))
	argument.AddArgument(options, fmt.Sprintf("-D CMAKE_C_COMPILER=%s", cCompiler))
	argument.AddArgument(options, fmt.Sprintf("-D CMAKE_LINKER=%s", linker))
	argument.AddArgument(options, fmt.Sprintf("-D CMAKE_BUILD_TYPE=%s", buildMode))
	argument.AddArgument(options, fmt.Sprintf("--graphviz=%s/output.dot", codeDirectory))
	argument.AddArgument(options, "-D CMAKE_C_INCLUDE_WHAT_YOU_USE=\"include-what-you-use;-w;-Xiwyu;--no_default_mappings\"")
}

func AddCommonBuildOptions(options *strings.Builder, buildDirectory string, threads int) {
	argument.AddArgument(options, fmt.Sprintf("--build %s", buildDirectory))
	argument.AddArgument(options, fmt.Sprintf("--parallel %d", threads))
}
