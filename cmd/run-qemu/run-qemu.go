package main

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/flags"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/bitfield/script"
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
	qemuOptions.WriteString("  -m 512\n")
	qemuOptions.WriteString("  -machine q35\n")
	qemuOptions.WriteString("  -no-reboot\n")
	qemuOptions.WriteString(fmt.Sprintf("  -drive \"format=raw,file=%s\"\n", osLocation))
	qemuOptions.WriteString(fmt.Sprintf("  -bios %s\n", uefiLocation))
	qemuOptions.WriteString("  -serial stdio\n")
	qemuOptions.WriteString("  -smp 1\n")
	qemuOptions.WriteString("  -usb\n")
	qemuOptions.WriteString("  -vga std\n")

	if verbose {
		qemuOptions.WriteString("  -d int,cpu_reset\n")
	}

	if debug {
		qemuOptions.WriteString("  -s -S\n")
		// NOTE: Ensure this is the same architecture as what you are trying to
		// build for :)))
		qemuOptions.WriteString("  -cpu Haswell-v4\n")
		qemuOptions.WriteString("  -accel \"tcg\"\n")
	} else {
		qemuOptions.WriteString("  -cpu host\n")
		qemuOptions.WriteString("  -enable-kvm\n")
	}

	fmt.Printf("%s%s%s\n%s", common.BOLD, QEMU_EXECUTABLE, common.RESET, qemuOptions.String())

	var finalCommand = fmt.Sprintf("%s\n%s", QEMU_EXECUTABLE, qemuOptions.String())
	script.Exec(finalCommand).Stdout()
}
