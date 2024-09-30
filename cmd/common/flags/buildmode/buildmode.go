package buildmode

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/converter"
	"cmd/common/flags"
	"flag"
	"fmt"
)

var PossibleBuildModes = [...]string{
	"Release",
	"Debug",
	// NOTE: these may be enabled later
	// "Profiling",
	// "Fuzzing"
}

const BUILD_MODE_LONG_FLAG = "build-mode"
const BUILD_MODE_SHORT_FLAG = "m"

func IsValidBuildMode(buildMode string) bool {
	for _, mode := range PossibleBuildModes {
		if buildMode == mode {
			return true
		}
	}
	return false
}

func DisplayBuildMode(defaultBuildMode string) {
	// Not sure why go doesnt understand string lengths of this one, but whatever
	var buildModeDescription = fmt.Sprintf("Set the build mode (%s%s%s)        ", common.WHITE,
		converter.ArrayIntoPrintableString(PossibleBuildModes[:]), common.RESET)
	flags.DisplayArgumentInput(BUILD_MODE_SHORT_FLAG, BUILD_MODE_LONG_FLAG, buildModeDescription, defaultBuildMode)
}

func AddBuildModeAsFlag(buildMode *string) {
	flag.StringVar(buildMode, BUILD_MODE_LONG_FLAG, *buildMode, "")
	flag.StringVar(buildMode, BUILD_MODE_SHORT_FLAG, *buildMode, "")
}

func DisplayBuildModeConfiguration(buildMode string) {
	configuration.DisplayStringArgument(BUILD_MODE_LONG_FLAG, buildMode)
}
