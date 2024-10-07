package common

import (
	"os"
	"path/filepath"
)

const BOLD = "\033[1m"
const RESET = "\033[0m"
const RED = "\033[31m"
const GREEN = "\033[32m"
const YELLOW = "\033[33m"
const BLUE = "\033[34m"
const PURPLE = "\033[35m"
const CYAN = "\033[36m"
const GRAY = "\033[37m"
const WHITE = "\033[97m"

const CMAKE_EXECUTABLE = "cmake"

func getRepoRoot() string {
	startDir, err := os.Getwd()
	if err != nil {
		return ""
	}

	dir := startDir
	for {
		if dir == "/" {
			return ""
		}

		homegrownPath := filepath.Join(dir, "homegrown")
		if _, err := os.Stat(homegrownPath); err == nil {
			return homegrownPath + "/"
		}

		dir = filepath.Dir(dir)
	}
}

var RepoRoot = getRepoRoot()

var PROJECT_FOLDER = RepoRoot + "projects/"

type Project int64

type ProjectStructure struct {
	Folder         string
	CodeFolder     string
	IsFreeStanding bool
}

var kernelFolder = PROJECT_FOLDER + "kernel/"
var interoperationFolder = PROJECT_FOLDER + "interoperation/"
var uefiImageCreatorFolder = PROJECT_FOLDER + "uefi-image-creator/"
var uefiFolder = PROJECT_FOLDER + "uefi/"
var imageBuilderFolder = PROJECT_FOLDER + "image-builder/"

// If you add a project, add an enum value and add it to the array below!
const (
	KERNEL Project = iota
	INTEROPERATION
	UEFI_IMAGE_CREATOR
	UEFI
	IMAGE_BUILDER
)

var PROJECTS = []ProjectStructure{
	ProjectStructure{
		Folder:         kernelFolder,
		CodeFolder:     kernelFolder + "code",
		IsFreeStanding: true,
	},
	ProjectStructure{
		Folder:         interoperationFolder,
		CodeFolder:     interoperationFolder + "code",
		IsFreeStanding: true,
	},
	ProjectStructure{
		Folder:         uefiImageCreatorFolder,
		CodeFolder:     uefiImageCreatorFolder + "code",
		IsFreeStanding: false,
	},
	ProjectStructure{
		Folder:         uefiFolder,
		CodeFolder:     uefiFolder + "code",
		IsFreeStanding: true,
	},
	ProjectStructure{
		Folder:         imageBuilderFolder,
		CodeFolder:     imageBuilderFolder + "code",
		IsFreeStanding: false,
	},
}
