package buildmode

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/converter"
	"cmd/common/flags"
	"flag"
	"fmt"
)

const BUILD_MODE_LONG_FLAG = "build-mode"
const BUILD_MODE_SHORT_FLAG = "m"

type BuildMode string

const (
	Release BuildMode = "Release"
	Debug   BuildMode = "Debug"
)

var PossibleBuildModes = []string{
	string(Release),
	string(Debug),
}

func IsValidBuildMode(mode string) bool {
	for _, validMode := range PossibleBuildModes {
		if mode == validMode {
			return true
		}
	}
	return false
}

func DefaultBuildMode() string {
	return string(Release)
}

func DisplayBuildMode(defaultBuildMode string) {
	// Not sure why go doesnt understand string lengths of this one, but whatever
	var buildModeDescription = fmt.Sprintf("Set the build mode (%s%s%s)        ", common.WHITE,
		converter.ArrayIntoPrintableString(PossibleBuildModes[:]), common.RESET)
	flags.DisplayArgumentInput(BUILD_MODE_SHORT_FLAG, BUILD_MODE_LONG_FLAG, buildModeDescription, string(defaultBuildMode))
}

func AddBuildModeAsFlag(buildMode *string) {
	flag.StringVar(buildMode, BUILD_MODE_LONG_FLAG, *buildMode, "")
	flag.StringVar(buildMode, BUILD_MODE_SHORT_FLAG, *buildMode, "")
}

func DisplayBuildModeConfiguration(buildMode string) {
	configuration.DisplayStringArgument(BUILD_MODE_LONG_FLAG, buildMode)
}
