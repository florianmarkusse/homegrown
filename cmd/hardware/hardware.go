package main

import (
	"cmd/clean/remove"
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/exit"
	"cmd/common/flags/buildmode"
	"cmd/common/uefiimage"
	"cmd/compile/builder"
	"fmt"
	"os"
)

func main() {
	remove.RemoveGeneratedFiles()

	var result = builder.Build(&builder.RunBuildArgs)
	if result == builder.Failure {
		fmt.Println("Failed to build project")
		os.Exit(exit.EXIT_TARGET_ERROR)
	}

	uefiimage.CreateUefiImage(buildmode.DefaultBuildMode())

	writeToUSBCommand := fmt.Sprintf("sudo dd bs=4M if=%s/%s of=/dev/sdc1 conv=notrunc", common.REPO_ROOT, common.FLOS_UEFI_IMAGE_FILE)
	argument.ExecCommand(writeToUSBCommand)
}
