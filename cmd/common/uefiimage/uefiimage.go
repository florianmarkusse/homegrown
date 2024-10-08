package uefiimage

import (
	"cmd/common/argument"
	"cmd/common/cmake"
	"fmt"
)

func CreateUefiImage(cCompiler string, buildMode string) {
	copyEfiCommand := fmt.Sprintf("find %s -executable -type f -name \"uefi\" -exec cp {} BOOTX64.EFI \\;", cmake.BuildDirectoryRoot(cmake.PROJECT_STRUCTURES[cmake.UEFI].CodeFolder, false, cCompiler))
	argument.ExecCommand(copyEfiCommand)

	copyKernelCommand := fmt.Sprintf("find %s -executable -type f -name \"kernel.bin\" -exec cp {} kernel.bin \\;", cmake.BuildDirectoryRoot(cmake.PROJECT_STRUCTURES[cmake.KERNEL].CodeFolder, false, cCompiler))
	argument.ExecCommand(copyKernelCommand)

	createTestImageCommand := fmt.Sprintf("find %s -type f -name \"uefi-image-creator\" -exec {} --data-size 32 -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin \\;", cmake.BuildDirectoryRoot(cmake.PROJECT_STRUCTURES[cmake.UEFI_IMAGE_CREATOR].CodeFolder, false, cCompiler))
	argument.ExecCommand(createTestImageCommand)
}
