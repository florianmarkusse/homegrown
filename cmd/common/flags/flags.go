package flags

import (
	"cmd/common"
	"fmt"
	"os"
	"path/filepath"
)

func DisplayUsage(requiredFlags string) {
	fmt.Printf("%s%sUsage%s:\n", common.BOLD, common.PURPLE, common.RESET)
	fmt.Printf("  %s %s %s[OPTIONS]%s\n", filepath.Base(os.Args[0]), requiredFlags, common.GRAY, common.RESET)
}

func DisplayRequiredFlags() {
	fmt.Printf("%s%sRequired%s Flags:\n", common.BOLD, common.YELLOW, common.RESET)
}

func DisplayOptionalFlags() {
	fmt.Printf("%sOptional%s Flags:\n", common.BOLD, common.RESET)
}

func DisplayExitCodes() {
	fmt.Printf("%s%sExit codes%s:\n", common.CYAN, common.BOLD, common.RESET)
}

func DisplayExamples() {
	fmt.Printf("%s%sExamples%s:\n", common.BLUE, common.BOLD, common.RESET)
}

func addDefaultValue(defaultValue string) string {
	return fmt.Sprintf("%sdefault%s %s%s%s", common.GRAY, common.RESET, common.YELLOW, defaultValue, common.RESET)
}

func addLongFlag(longFlag string) string {
	return fmt.Sprintf("--%-15s", longFlag)
}

func addShortFlag(shortFlag string) string {
	return fmt.Sprintf("-%s", shortFlag)
}

func addDescription(description string) string {
	return fmt.Sprintf("%s%-60s%s", common.GRAY, description, common.RESET)
}

func DisplayLongFlagArgumentInput(longFlag string, description string, defaultValue string) {
	fmt.Printf("      %s %s %s\n", addLongFlag(longFlag), addDescription(description), addDefaultValue(defaultValue))
}

func DisplayArgumentInput(shortFlag string, longFlag string, description string, defaultValue string) {
	fmt.Printf("  %s, %s %s %s\n", addShortFlag(shortFlag), addLongFlag(longFlag), addDescription(description), addDefaultValue(defaultValue))
}

func DisplayNoDefaultArgumentInput(shortFlag string, longFlag string, description string) {
	fmt.Printf("  %s, %s %s\n", addShortFlag(shortFlag), addLongFlag(longFlag), addDescription(description))
}

func DisplayExitCode(code uint8, description string) {
	fmt.Printf("  %-21d %s\n", code, addDescription(description))
}
