package uefiimage

import (
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/cmake"
	"fmt"
)

func CreateUefiImage(cCompiler string, buildMode string) {
	copyEfiCommand := fmt.Sprintf("find %s -executable -type f -name \"uefi-%s\" -exec cp {} BOOTX64.EFI \\;", cmake.BuildDirectoryRoot(common.UEFI_CODE_FOLDER, false, cCompiler), buildMode)
	argument.ExecCommand(copyEfiCommand)

	copyKernelCommand := fmt.Sprintf("find %s -executable -type f -name \"kernel-%s.bin\" -exec cp {} kernel.bin \\;", cmake.BuildDirectoryRoot(common.KERNEL_CODE_FOLDER, false, cCompiler), buildMode)
	argument.ExecCommand(copyKernelCommand)

	createTestImageCommand := fmt.Sprintf("find %s -type f -name \"uefi-image-creator-%s\" -exec {} --data-size 32 -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin \\;", cmake.BuildDirectoryRoot(common.UEFI_IMAGE_CREATOR_CODE_FOLDER, false, cCompiler), buildMode)
	argument.ExecCommand(createTestImageCommand)
}
