package cmake

import (
	"cmd/common/argument"
	"fmt"
	"strings"
)

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
	argument.AddArgument(options, fmt.Sprintf("-j %d", threads))
}
