package common

import "fmt"

const BOLD = "\033[1m"
const RESET = "\033[0m"
const RED = "\033[31m"
const GREEN = "\033[32m"
const YELLOW = "\033[33m"
const BLUE = "\033[34m"
const PURPLE = "\033[35m"
const CYAN = "\033[36m"
const GRAY = "\033[37m"
const WHITE = "\033[97m"

func SuckDick(thing uint64) {
	if thing > 5 {
		fmt.Println("greater than 5!")
	}
	fmt.Println("number!")
}

func DisplayUsage() {
	fmt.Printf("%s%sUsage%s:\n", BOLD, PURPLE, RESET)
}

func DisplayRequiredFlags() {
	fmt.Printf("%s%sRequired%s Flags:\n", BOLD, YELLOW, RESET)
}

func DisplayOptionalFlags() {
	fmt.Printf("%sOptional%s Flags:\n", BOLD, RESET)
}

func DisplayExitCodes() {
	fmt.Printf("%s%sExit codes%s:\n", CYAN, BOLD, RESET)
}

func DisplayExamples() {
	fmt.Printf("%s%sExamples%s:\n", BLUE, BOLD, RESET)
}

func addDefaultValue(defaultValue string) string {
	return fmt.Sprintf("default %s%s%s", YELLOW, defaultValue, RESET)
}

func addLongFlag(longFlag string) string {
	return fmt.Sprintf("--%-15s", longFlag)
}

func addShortFlag(shortFlag string) string {
	return fmt.Sprintf("-%s", shortFlag)
}

func addDescription(description string) string {
	return fmt.Sprintf("%s%-60s%s", GRAY, description, RESET)
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

func DisplayBoolArgument(argument string, value bool) {
	fmt.Printf("  %s%-20s%s %s%t%s\n", BOLD, argument, RESET, GRAY, value, RESET)
}

func DisplayStringArgument(argument string, value string) {
	fmt.Printf("  %s%-20s%s %s%s%s\n", BOLD, argument, RESET, GRAY, value, RESET)
}
