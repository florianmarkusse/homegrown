package main

import (
	"cmd/clean/remove"
	"cmd/common"
	"cmd/common/argument"
	"cmd/common/exit"
	"cmd/common/uefiimage"
	"cmd/compile/projects"
	"fmt"
	"os"
)

func main() {
	remove.RemoveGeneratedFiles()

	var result = projects.Build(&projects.DefaultBuildArgs)
	if result == projects.Failure {
		fmt.Println("Failed to build project")
		os.Exit(exit.EXIT_TARGET_ERROR)
	}

	uefiimage.CreateUefiImage()

	writeToUSBCommand := fmt.Sprintf("sudo dd bs=4M if=%s/test.hdd of=/dev/sdc1 conv=notrunc", common.RepoRoot)
	argument.ExecCommand(writeToUSBCommand)
}
