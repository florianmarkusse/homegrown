package uefiimage

import (
	"cmd/common/cmd"
	"fmt"
	"log"
)

func CreateUefiImage(buildMode string) {
	copyEfiCommand := fmt.Sprintf("find projects/uefi/code/build -executable -type f -name \"uefi-%s\" -exec cp {} BOOTX64.EFI \\;", buildMode)
	if err := cmd.ExecCommand(copyEfiCommand); err != nil {
		log.Fatalf("Failed to copy EFI file: %v", err)
	}

	copyKernelCommand := fmt.Sprintf("find projects/kernel/code/build -executable -type f -name \"kernel-%s.bin\" -exec cp {} kernel.bin \\;", buildMode)
	if err := cmd.ExecCommand(copyKernelCommand); err != nil {
		log.Fatalf("Failed to copy kernel file: %v", err)
	}

	createTestImageCommand := fmt.Sprintf("find projects/uefi-image-creator/code/build -type f -name \"uefi-image-creator-%s\" -exec {} --data-size 32 -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin \\;", buildMode)
	if err := cmd.ExecCommand(createTestImageCommand); err != nil {
		log.Fatalf("Failed to create UEFI image: %v", err)
	}
}
