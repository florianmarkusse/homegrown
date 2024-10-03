package main

import (
	"cmd/clean/remove"
	"cmd/common"
	"cmd/common/cmd"
	"cmd/common/exit"
	"cmd/common/uefiimage"
	"cmd/compile/projects"
	"fmt"
	"log"
	"os"
)

func main() {
	remove.RemoveGeneratedFiles()

	var result = projects.Build(&projects.DefaultBuildArgs)
	if result == projects.Failure {
		fmt.Println("Failed to build project")
		os.Exit(exit.EXIT_TARGET_ERROR)
	}

	uefiimage.CreateUefiImage(projects.DefaultBuildArgs.BuildMode)

	writeToUSBCommand := fmt.Sprintf("sudo dd bs=4M if=%s/test.hdd of=/dev/sdc1 conv=notrunc", common.RepoRoot)
	if err := cmd.ExecCommand(writeToUSBCommand); err != nil {
		log.Fatalf("Failed to write to USB: %v", err)
	}
}
