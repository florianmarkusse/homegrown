package main

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/exit"
	"cmd/common/flags"
	"cmd/common/flags/help"
	"cmd/run-qemu/qemu"
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

const OS_LOCATION_LONG_FLAG = "os-location"
const OS_LOCATION_SHORT_FLAG = "o"

const EFI_LOCATION_LONG_FLAG = "efi-location"
const EFI_LOCATION_SHORT_FLAG = "u"

const VERBOSE_LONG_FLAG = "verbose"
const VERBOSE_SHORT_FLAG = "v"

const DEBUG_LONG_FLAG = "debug"
const DEBUG_SHORT_FLAG = "d"

var qemuArgs = qemu.DefaultQemuArgs

var isHelp = false

func usage() {
	var usageFlags = ""
	flags.DisplayUsage(usageFlags)
	fmt.Printf("\n")
	flags.DisplayOptionalFlags()
	flags.DisplayArgumentInput(OS_LOCATION_SHORT_FLAG, OS_LOCATION_LONG_FLAG, "Set the OS (.hdd) location", qemuArgs.OsLocation)
	flags.DisplayArgumentInput(EFI_LOCATION_SHORT_FLAG, EFI_LOCATION_LONG_FLAG, "set the UEFI (.bin) location to emulate UEFI environment", qemuArgs.UefiLocation)
	flags.DisplayArgumentInput(VERBOSE_SHORT_FLAG, VERBOSE_LONG_FLAG, "Enable verbose QEMU", fmt.Sprint(qemuArgs.Verbose))
	flags.DisplayArgumentInput(DEBUG_SHORT_FLAG, DEBUG_LONG_FLAG, "Wait for gdb to connect to port 1234 before running", fmt.Sprint(qemuArgs.Debug))
	help.DisplayHelp()
	fmt.Printf("\n")
	exit.DisplayExitCodes()
	exit.DisplayExitCode(exit.EXIT_SUCCESS)
	exit.DisplayExitCode(exit.EXIT_MISSING_ARGUMENT)
	exit.DisplayExitCode(exit.EXIT_CLI_PARSING_ERROR)
	fmt.Printf("\n")
	flags.DisplayExamples()
	fmt.Printf("  %s --%s %s --%s %s\n", filepath.Base(os.Args[0]), OS_LOCATION_LONG_FLAG, common.FLOS_UEFI_IMAGE_FILE, EFI_LOCATION_LONG_FLAG, common.BIOS_FILE)
	fmt.Printf("  %s -%s=%s -%s %s -%s --%s\n", filepath.Base(os.Args[0]), OS_LOCATION_LONG_FLAG, common.FLOS_UEFI_IMAGE_FILE, EFI_LOCATION_SHORT_FLAG, common.BIOS_FILE, VERBOSE_SHORT_FLAG, DEBUG_LONG_FLAG)
	fmt.Printf("\n")
}

func main() {
	flag.StringVar(&qemuArgs.OsLocation, OS_LOCATION_LONG_FLAG, qemuArgs.OsLocation, "")
	flag.StringVar(&qemuArgs.OsLocation, OS_LOCATION_SHORT_FLAG, qemuArgs.OsLocation, "")

	flag.StringVar(&qemuArgs.UefiLocation, EFI_LOCATION_LONG_FLAG, qemuArgs.UefiLocation, "")
	flag.StringVar(&qemuArgs.UefiLocation, EFI_LOCATION_SHORT_FLAG, qemuArgs.UefiLocation, "")

	flag.BoolVar(&qemuArgs.Verbose, VERBOSE_LONG_FLAG, qemuArgs.Verbose, "")
	flag.BoolVar(&qemuArgs.Verbose, VERBOSE_SHORT_FLAG, qemuArgs.Verbose, "")

	flag.BoolVar(&qemuArgs.Debug, DEBUG_LONG_FLAG, qemuArgs.Debug, "")
	flag.BoolVar(&qemuArgs.Debug, DEBUG_SHORT_FLAG, qemuArgs.Debug, "")

	help.AddHelpAsFlag(&isHelp)

	flag.Usage = usage
	flag.Parse()

	var showHelpAndExit = false

	if isHelp {
		showHelpAndExit = true
	}

	if showHelpAndExit {
		usage()
		if isHelp {
			os.Exit(exit.EXIT_SUCCESS)
		} else {
			os.Exit(exit.EXIT_MISSING_ARGUMENT)
		}
	}

	configuration.DisplayConfiguration()
	configuration.DisplayStringArgument(OS_LOCATION_LONG_FLAG, qemuArgs.OsLocation)
	configuration.DisplayStringArgument(EFI_LOCATION_LONG_FLAG, qemuArgs.UefiLocation)
	configuration.DisplayBoolArgument(VERBOSE_LONG_FLAG, qemuArgs.Verbose)
	configuration.DisplayBoolArgument(DEBUG_LONG_FLAG, qemuArgs.Debug)
	fmt.Printf("\n")

	qemu.Run(&qemuArgs)
}
