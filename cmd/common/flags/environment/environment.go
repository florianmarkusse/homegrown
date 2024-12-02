package environment

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/converter"
	"cmd/common/flags"
	"flag"
	"fmt"
)

const ENVIRONMENT_LONG_FLAG = "environment"
const ENVIRONMENT_SHORT_FLAG = "n"

type Environment string

const (
	Freestanding Environment = "freestanding"
	Posix        Environment = "posix"
)

var PossibleEnvironments = []string{
	string(Freestanding),
	string(Posix),
}

func IsValidEnvironment(mode string) bool {
	for _, validEnvironment := range PossibleEnvironments {
		if mode == validEnvironment {
			return true
		}
	}
	return false
}

func DisplayEnvironment() {
	// Not sure why go doesnt understand string lengths of this one, but whatever
	var buildModeDescription = fmt.Sprintf("Set the environment (%s%s%s)                    ", common.WHITE,
		converter.ArrayIntoPrintableString(PossibleEnvironments[:]), common.RESET)
	flags.DisplayArgumentInput(ENVIRONMENT_SHORT_FLAG, ENVIRONMENT_LONG_FLAG, buildModeDescription, "_CONFIGURED SEPARATELY BY EACH PROJECT_")
}

func AddEnvironmentAsFlag(environment *string) {
	flag.StringVar(environment, ENVIRONMENT_LONG_FLAG, *environment, "")
	flag.StringVar(environment, ENVIRONMENT_SHORT_FLAG, *environment, "")
}

func DisplayEnvironmentConfiguration(environment string) {
	if environment == "" {
		configuration.DisplayStringArgument(ENVIRONMENT_LONG_FLAG, "_PROJECT DEFAULT_")
		return
	}
	configuration.DisplayStringArgument(ENVIRONMENT_LONG_FLAG, environment)
}
