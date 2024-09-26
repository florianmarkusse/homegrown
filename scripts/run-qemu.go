package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/bitfield/script"
)

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

const OS_LOCATION_LONG_FLAG = "os-location"
const OS_LOCATION_SHORT_FLAG = "o"

const UEFI_LOCATION_LONG_FLAG = "uefi-location"
const UEFI_LOCATION_SHORT_FLAG = "u"

const VERBOSE_LONG_FLAG = "verbose"
const VERBOSE_SHORT_FLAG = "v"

const DEBUG_LONG_FLAG = "debug"
const DEBUG_SHORT_FLAG = "d"

const HELP_LONG_FLAG = "help"
const HELP_SHORT_FLAG = "h"

const EXIT_SUCCESS = 0
const EXIT_GENERAL_ERROR = 1
const EXIT_MISSING_ARGUMENT = 2

func displayArgumentInput(shortFlag string, longFlag string, description string) {
	fmt.Printf("  -%s, --%-20s%s%s%s\n", shortFlag, longFlag, GRAY, description, RESET)
}

func usage() {
	fmt.Printf("Usage: %s%s %s <os_location> --uefi-location <uefi_location> [OPTIONS]\n", filepath.Base(os.Args[0]), GRAY, OS_LOCATION_LONG_FLAG)
	fmt.Printf("%s%sRequired%s Options:\n", BOLD, YELLOW, RESET)
	displayArgumentInput(OS_LOCATION_SHORT_FLAG, OS_LOCATION_LONG_FLAG, "Set the OS (.hdd) location")
	displayArgumentInput(UEFI_LOCATION_SHORT_FLAG, UEFI_LOCATION_LONG_FLAG, "set the UEFI (.bin) location to emulate UEFI environment (like OVMF firmware)")
	fmt.Printf("\n")
	fmt.Printf("%sOptional%s Options:\n", BOLD, RESET)
	displayArgumentInput(VERBOSE_SHORT_FLAG, VERBOSE_LONG_FLAG, "Enable verbose QEMU")
	displayArgumentInput(DEBUG_SHORT_FLAG, DEBUG_LONG_FLAG, "Wait for gdb to connect to port 1234 before running")
	displayArgumentInput(HELP_SHORT_FLAG, HELP_LONG_FLAG, "Display this help message")
	fmt.Printf("\n")
	fmt.Printf("%s%sExamples%s:\n", BLUE, BOLD, RESET)
	fmt.Printf("  %s --%s test.hdd --%s bios.bin\n", filepath.Base(os.Args[0]), UEFI_LOCATION_LONG_FLAG, OS_LOCATION_LONG_FLAG)
	fmt.Printf("  %s -%s=test.hdd -%s bios.bin -%s --%s\n", filepath.Base(os.Args[0]), UEFI_LOCATION_SHORT_FLAG, OS_LOCATION_LONG_FLAG, VERBOSE_SHORT_FLAG, DEBUG_LONG_FLAG)
}

var osLocation string
var uefiLocation string
var verbose bool
var debug bool
var help bool

const QEMU_EXECUTABLE = "qemu-system-x86_64"

func displayBoolArgument(argument string, value bool) {
	fmt.Printf("  %s%-20s%s%s%t%s\n", BOLD, argument, RESET, GRAY, value, RESET)
}

func displayStringArgument(argument string, value string) {
	fmt.Printf("  %s%-20s%s%s%s%s\n", BOLD, argument, RESET, GRAY, value, RESET)
}

func main() {
	flag.StringVar(&osLocation, OS_LOCATION_LONG_FLAG, "", "")
	flag.StringVar(&osLocation, OS_LOCATION_SHORT_FLAG, "", "")

	flag.StringVar(&uefiLocation, UEFI_LOCATION_LONG_FLAG, "", "")
	flag.StringVar(&uefiLocation, UEFI_LOCATION_SHORT_FLAG, "", "")

	flag.BoolVar(&verbose, VERBOSE_LONG_FLAG, false, "")
	flag.BoolVar(&verbose, VERBOSE_SHORT_FLAG, false, "")

	flag.BoolVar(&debug, DEBUG_LONG_FLAG, false, "")
	flag.BoolVar(&debug, DEBUG_SHORT_FLAG, false, "")

	flag.BoolVar(&help, HELP_LONG_FLAG, false, "")
	flag.BoolVar(&help, HELP_SHORT_FLAG, false, "")

	flag.Parse()

	var showHelpAndExit = false
	if osLocation == "" {
		showHelpAndExit = true
	}
	if uefiLocation == "" {
		showHelpAndExit = true
	}
	if help {
		showHelpAndExit = true
	}

	if showHelpAndExit {
		usage()
		if help {
			os.Exit(0)
		} else {
			os.Exit(2)
		}
	}

	fmt.Printf("%s%sConfiguration%s:\n", BOLD, YELLOW, RESET)
	displayStringArgument(OS_LOCATION_LONG_FLAG, osLocation)
	displayStringArgument(UEFI_LOCATION_LONG_FLAG, uefiLocation)
	displayBoolArgument(VERBOSE_LONG_FLAG, verbose)
	displayBoolArgument(DEBUG_LONG_FLAG, debug)
	displayBoolArgument(HELP_LONG_FLAG, help)
	fmt.Printf("\n")

	var qemuOptions = make([]string, 0)
	qemuOptions = append(qemuOptions, "-m 512")
	qemuOptions = append(qemuOptions, "-machine q35")
	qemuOptions = append(qemuOptions, "-no-reboot")
	qemuOptions = append(qemuOptions, fmt.Sprintf("-drive \"format=raw,file=%s\"", osLocation))
	qemuOptions = append(qemuOptions, fmt.Sprintf("-bios %s", uefiLocation))
	qemuOptions = append(qemuOptions, "-serial stdio")
	qemuOptions = append(qemuOptions, "-smp 1")
	qemuOptions = append(qemuOptions, "-usb")
	qemuOptions = append(qemuOptions, "-vga std")

	if verbose {
		qemuOptions = append(qemuOptions, "-d int,cpu_reset")
	}

	if debug {
		qemuOptions = append(qemuOptions, "-s -S")
		// NOTE: Ensure this is the same architecture as what you are trying to
		// build for :)))
		qemuOptions = append(qemuOptions, "-cpu Haswall-v4")
		qemuOptions = append(qemuOptions, "-accel \"tcg\"")
	} else {
		qemuOptions = append(qemuOptions, "-cpu host")
		qemuOptions = append(qemuOptions, "-enable-kvm")
	}

	finalCommand := strings.Builder{}
	finalCommand.WriteString(QEMU_EXECUTABLE)
	finalCommand.WriteString(" ")

	fmt.Printf("%s%s%s\n", BOLD, QEMU_EXECUTABLE, RESET)
	for _, argument := range qemuOptions {
		fmt.Printf("  %s\n", argument)
		finalCommand.WriteString(argument)
		finalCommand.WriteString(" ")
	}

	script.Exec(finalCommand.String()).Stdout()
}
