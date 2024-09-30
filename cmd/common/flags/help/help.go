package help

import (
	"cmd/common/flags"
	"flag"
)

const HELP_LONG_FLAG = "help"
const HELP_SHORT_FLAG = "h"

func DisplayHelp() {
	flags.DisplayNoDefaultArgumentInput(HELP_SHORT_FLAG, HELP_LONG_FLAG, "Display this help message")
}

func AddHelpAsFlag(isHelp *bool) {
	flag.BoolVar(isHelp, HELP_LONG_FLAG, *isHelp, "")
	flag.BoolVar(isHelp, HELP_SHORT_FLAG, *isHelp, "")
}
