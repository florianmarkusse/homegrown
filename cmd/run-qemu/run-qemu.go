package main

import (
	"cmd/common"
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
	common.DisplayUsage()
	fmt.Printf("  %s %s%s <os_location> --uefi-location <uefi_location> [OPTIONS]%s\n", filepath.Base(os.Args[0]), common.GRAY, OS_LOCATION_LONG_FLAG, common.RESET)
	fmt.Printf("\n")
	common.DisplayRequiredFlags()
	common.DisplayArgumentInput(OS_LOCATION_SHORT_FLAG, OS_LOCATION_LONG_FLAG, "Set the OS (.hdd) location")
	common.DisplayArgumentInput(UEFI_LOCATION_SHORT_FLAG, UEFI_LOCATION_LONG_FLAG, "set the UEFI (.bin) location to emulate UEFI environment (like OVMF firmware)")
	common.DisplayOptionalFlags()
	common.DisplayArgumentInput(VERBOSE_SHORT_FLAG, VERBOSE_LONG_FLAG, "Enable verbose QEMU")
	common.DisplayArgumentInput(DEBUG_SHORT_FLAG, DEBUG_LONG_FLAG, "Wait for gdb to connect to port 1234 before running")
	common.DisplayArgumentInput(HELP_SHORT_FLAG, HELP_LONG_FLAG, "Display this help message")
	fmt.Printf("\n")
	common.DisplayExitCodes()
	common.DisplayExitCode(EXIT_SUCCESS, "Success")
	common.DisplayExitCode(EXIT_MISSING_ARGUMENT, "Incorrect argument(s)")
	fmt.Printf("\n")
	common.DisplayExamples()
	fmt.Printf("  %s --%s test.hdd --%s bios.bin\n", filepath.Base(os.Args[0]), UEFI_LOCATION_LONG_FLAG, OS_LOCATION_LONG_FLAG)
	fmt.Printf("  %s -%s=test.hdd -%s bios.bin -%s --%s\n", filepath.Base(os.Args[0]), UEFI_LOCATION_SHORT_FLAG, OS_LOCATION_LONG_FLAG, VERBOSE_SHORT_FLAG, DEBUG_LONG_FLAG)
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

	fmt.Printf("%s%sConfiguration%s:\n", common.BOLD, common.YELLOW, common.RESET)
	common.DisplayStringArgument(OS_LOCATION_LONG_FLAG, osLocation)
	common.DisplayStringArgument(UEFI_LOCATION_LONG_FLAG, uefiLocation)
	common.DisplayBoolArgument(VERBOSE_LONG_FLAG, verbose)
	common.DisplayBoolArgument(DEBUG_LONG_FLAG, debug)
	common.DisplayBoolArgument(HELP_LONG_FLAG, help)
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
		qemuOptions = append(qemuOptions, "-cpu Haswell-v4")
		qemuOptions = append(qemuOptions, "-accel \"tcg\"")
	} else {
		qemuOptions = append(qemuOptions, "-cpu host")
		qemuOptions = append(qemuOptions, "-enable-kvm")
	}

	finalCommand := strings.Builder{}
	finalCommand.WriteString(QEMU_EXECUTABLE)
	finalCommand.WriteString(" ")

	fmt.Printf("%s%s%s\n", common.BOLD, QEMU_EXECUTABLE, common.RESET)
	for _, argument := range qemuOptions {
		fmt.Printf("  %s\n", argument)
		finalCommand.WriteString(argument)
		finalCommand.WriteString(" ")
	}

	script.Exec(finalCommand.String()).Stdout()
}
