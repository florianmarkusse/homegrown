package uefiimage

import (
	"cmd/common/argument"
	"cmd/common/project"
	"fmt"
)

func CreateUefiImage(buildMode string) {
	copyEfiCommand := fmt.Sprintf("find %s -executable -type f -name \"%s\" -exec cp {} BOOTX64.EFI \\;", project.BuildDirectoryRoot(project.PROJECT_STRUCTURES[project.EFI], buildMode), project.EFI)
	argument.ExecCommand(copyEfiCommand)

	copyKernelCommand := fmt.Sprintf("find %s -executable -type f -name \"kernel.bin\" -exec cp {} kernel.bin \\;", project.BuildDirectoryRoot(project.PROJECT_STRUCTURES[project.KERNEL], buildMode))
	argument.ExecCommand(copyKernelCommand)

	createTestImageCommand := fmt.Sprintf("find %s -type f -name \"uefi-image-creator\" -exec {} --data-size 32 -ae /EFI/BOOT/ BOOTX64.EFI -ad kernel.bin \\;", project.BuildDirectoryRoot(project.PROJECT_STRUCTURES[project.UEFI_IMAGE_CREATOR], buildMode))
	argument.ExecCommand(createTestImageCommand)
}
