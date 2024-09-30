package uefiimage

import (
	"fmt"

	"github.com/bitfield/script"
)

func CreateUefiImage(buildMode string) {
	var copyEfiCommand = fmt.Sprintf("find projects/uefi/code/build -executable -type f -name \"uefi-%s\" -exec cp {} BOOTX64.EFI \\;", buildMode)
	script.Exec(copyEfiCommand)

	var copyKernelCommand = fmt.Sprintf("find projects/kernel/code/build -executable -type f -name \"kernel-%s.bin\" -exec cp {} kernel.bin \\;", buildMode)
	script.Exec(copyKernelCommand)

	var createTestImageCommand = fmt.Sprintf("find projects/uefi-image-creator/code/build -type f -name \"uefi-image-creator-%s\" -exec {} --data-size 32 -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin \\;", buildMode)
	script.Exec(createTestImageCommand)
}
