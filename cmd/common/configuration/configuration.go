package configuration

import (
	"cmd/common"
	"fmt"
)

func DisplayConfiguration() {
	fmt.Printf("%s%sConfiguration%s:\n", common.BOLD, common.YELLOW, common.RESET)
}

func DisplayIntArgument(argument string, value int) {
	fmt.Printf("  %s%-20s%s %s%d%s\n", common.BOLD, argument, common.RESET, common.GRAY, value, common.RESET)
}

func DisplayBoolArgument(argument string, value bool) {
	fmt.Printf("  %s%-20s%s %s%t%s\n", common.BOLD, argument, common.RESET, common.GRAY, value, common.RESET)
}

func DisplayStringArgument(argument string, value string) {
	fmt.Printf("  %s%-20s%s %s%s%s\n", common.BOLD, argument, common.RESET, common.GRAY, value, common.RESET)
}
