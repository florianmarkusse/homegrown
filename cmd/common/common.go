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
			return homegrownPath
		}

		dir = filepath.Dir(dir)
	}
}

var FLOS_EFI_FILE = "FLOS_EFI.efi"
var FLOS_KERNEL_FILE = "FLOS_KERNEL.bin"
var FLOS_UEFI_IMAGE_FILE = "FLOS_UEFI_IMAGE.hdd"
var BIOS_FILE = "bios.bin"

var REPO_ROOT = getRepoRoot()
var REPO_DEPENDENCIES = REPO_ROOT + "/dependencies"
var REPO_PROJECTS = REPO_ROOT + "/projects"
