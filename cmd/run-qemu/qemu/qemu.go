package qemu

import (
	"cmd/common"
	"cmd/common/argument"
	"fmt"
	"strings"
)

const QEMU_EXECUTABLE = "qemu-system-x86_64"

type QemuArgs struct {
	OsLocation   string
	UefiLocation string
	Verbose      bool
	Debug        bool
}

var DefaultQemuArgs = QemuArgs{
	OsLocation:   common.REPO_ROOT + "/" + common.FLOS_UEFI_IMAGE_FILE,
	UefiLocation: common.REPO_ROOT + "/" + common.BIOS_FILE,
	Verbose:      false,
	Debug:        false,
}

func Run(args *QemuArgs) {
	qemuOptions := strings.Builder{}
	argument.AddArgument(&qemuOptions, "-m 512")
	argument.AddArgument(&qemuOptions, "-machine q35")
	argument.AddArgument(&qemuOptions, "-no-reboot")
	argument.AddArgument(&qemuOptions, fmt.Sprintf("-drive \"format=raw,file=%s\"", args.OsLocation))
	argument.AddArgument(&qemuOptions, fmt.Sprintf("-bios %s", args.UefiLocation))
	argument.AddArgument(&qemuOptions, "-serial stdio")
	argument.AddArgument(&qemuOptions, "-smp 1")
	argument.AddArgument(&qemuOptions, "-usb")
	argument.AddArgument(&qemuOptions, "-vga std")

	if args.Verbose {
		argument.AddArgument(&qemuOptions, "-d int,cpu_reset")
	}

	if args.Debug {
		argument.AddArgument(&qemuOptions, "-s -S")
		// NOTE: Ensure this is the same architecture as what you are trying to
		// build for :)))
		argument.AddArgument(&qemuOptions, "-cpu Haswell-v4")
		argument.AddArgument(&qemuOptions, "-accel \"tcg\"")
	} else {
		argument.AddArgument(&qemuOptions, "-cpu host")
		argument.AddArgument(&qemuOptions, "-enable-kvm")
	}

	argument.ExecCommand(fmt.Sprintf("%s %s", QEMU_EXECUTABLE, qemuOptions.String()))
}
