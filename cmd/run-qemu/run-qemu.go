package main

import (
	"cmd/common/argument"
	"cmd/common/configuration"
	"cmd/common/flags"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

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
const EXIT_MISSING_ARGUMENT = 1
const EXIT_CLI_PARSING_ERROR = 2

func usage() {
	var usageFlags = fmt.Sprintf("--%s <os_location> --%s <uefi_location>", OS_LOCATION_LONG_FLAG, UEFI_LOCATION_LONG_FLAG)
	flags.DisplayUsage(usageFlags)
	fmt.Printf("\n")
	flags.DisplayRequiredFlags()
	flags.DisplayNoDefaultArgumentInput(OS_LOCATION_SHORT_FLAG, OS_LOCATION_LONG_FLAG, "Set the OS (.hdd) location")
	flags.DisplayNoDefaultArgumentInput(UEFI_LOCATION_SHORT_FLAG, UEFI_LOCATION_LONG_FLAG, "set the UEFI (.bin) location to emulate UEFI environment")
	flags.DisplayOptionalFlags()
	flags.DisplayArgumentInput(VERBOSE_SHORT_FLAG, VERBOSE_LONG_FLAG, "Enable verbose QEMU", fmt.Sprint(verbose))
	flags.DisplayArgumentInput(DEBUG_SHORT_FLAG, DEBUG_LONG_FLAG, "Wait for gdb to connect to port 1234 before running", fmt.Sprint(debug))
	flags.DisplayNoDefaultArgumentInput(HELP_SHORT_FLAG, HELP_LONG_FLAG, "Display this help message")
	fmt.Printf("\n")
	flags.DisplayExitCodes()
	flags.DisplayExitCode(EXIT_SUCCESS, "Success")
	flags.DisplayExitCode(EXIT_MISSING_ARGUMENT, "Incorrect argument(s)")
	flags.DisplayExitCode(EXIT_CLI_PARSING_ERROR, "CLI parsing error")
	fmt.Printf("\n")
	flags.DisplayExamples()
	fmt.Printf("  %s --%s test.hdd --%s bios.bin\n", filepath.Base(os.Args[0]), OS_LOCATION_LONG_FLAG, UEFI_LOCATION_LONG_FLAG)
	fmt.Printf("  %s -%s=test.hdd -%s bios.bin -%s --%s\n", filepath.Base(os.Args[0]), OS_LOCATION_LONG_FLAG, UEFI_LOCATION_SHORT_FLAG, VERBOSE_SHORT_FLAG, DEBUG_LONG_FLAG)
	fmt.Printf("\n")
}

var osLocation string
var uefiLocation string
var verbose = false
var debug = false
var help = false

const QEMU_EXECUTABLE = "qemu-system-x86_64"

func main() {
	flag.StringVar(&osLocation, OS_LOCATION_LONG_FLAG, "", "")
	flag.StringVar(&osLocation, OS_LOCATION_SHORT_FLAG, "", "")

	flag.StringVar(&uefiLocation, UEFI_LOCATION_LONG_FLAG, "", "")
	flag.StringVar(&uefiLocation, UEFI_LOCATION_SHORT_FLAG, "", "")

	flag.BoolVar(&verbose, VERBOSE_LONG_FLAG, verbose, "")
	flag.BoolVar(&verbose, VERBOSE_SHORT_FLAG, verbose, "")

	flag.BoolVar(&debug, DEBUG_LONG_FLAG, debug, "")
	flag.BoolVar(&debug, DEBUG_SHORT_FLAG, debug, "")

	flag.BoolVar(&help, HELP_LONG_FLAG, help, "")
	flag.BoolVar(&help, HELP_SHORT_FLAG, help, "")

	flag.Usage = usage
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
			os.Exit(EXIT_SUCCESS)
		} else {
			os.Exit(EXIT_MISSING_ARGUMENT)
		}
	}

	configuration.DisplayConfiguration()
	configuration.DisplayStringArgument(OS_LOCATION_LONG_FLAG, osLocation)
	configuration.DisplayStringArgument(UEFI_LOCATION_LONG_FLAG, uefiLocation)
	configuration.DisplayBoolArgument(VERBOSE_LONG_FLAG, verbose)
	configuration.DisplayBoolArgument(DEBUG_LONG_FLAG, debug)
	configuration.DisplayBoolArgument(HELP_LONG_FLAG, help)
	fmt.Printf("\n")

	qemuOptions := strings.Builder{}
	argument.AddArgument(&qemuOptions, "-m 512")
	argument.AddArgument(&qemuOptions, "-machine q35")
	argument.AddArgument(&qemuOptions, "-no-reboot")
	argument.AddArgument(&qemuOptions, fmt.Sprintf("-drive \"format=raw,file=%s\"", osLocation))
	argument.AddArgument(&qemuOptions, fmt.Sprintf("-bios %s", uefiLocation))
	argument.AddArgument(&qemuOptions, "-serial stdio")
	argument.AddArgument(&qemuOptions, "-smp 1")
	argument.AddArgument(&qemuOptions, "-usb")
	argument.AddArgument(&qemuOptions, "-vga std")

	if verbose {
		argument.AddArgument(&qemuOptions, "-d int,cpu_reset")
	}

	if debug {
		argument.AddArgument(&qemuOptions, "-s -S")
		// NOTE: Ensure this is the same architecture as what you are trying to
		// build for :)))
		argument.AddArgument(&qemuOptions, "-cpu Haswell-v4")
		argument.AddArgument(&qemuOptions, "-accel \"tcg\"")
	} else {
		argument.AddArgument(&qemuOptions, "-cpu host")
		argument.AddArgument(&qemuOptions, "-enable-kvm")
	}

	argument.RunCommand(QEMU_EXECUTABLE, qemuOptions.String())
}
