package error

import (
	"cmd/common"
	"cmd/common/flags"
	"fmt"
)

const EXIT_SUCCESS = 0
const EXIT_MISSING_ARGUMENT = 1
const EXIT_CLI_PARSING_ERROR = 2
const EXIT_TARGET_ERROR = 3

var errorToString = [...]string{
	"Success",
	"Incorrect argument(s)",
	"CLI parsing error",
	"Targets to build error",
}

func DisplayExitCodes() {
	fmt.Printf("%s%sExit codes%s:\n", common.CYAN, common.BOLD, common.RESET)
}

func DisplayExitCode(exitCode uint8) {
	flags.DisplayExitCode(exitCode, errorToString[exitCode])
}
