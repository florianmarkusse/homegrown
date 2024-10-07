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
			return homegrownPath + "/"
		}

		dir = filepath.Dir(dir)
	}
}

var RepoRoot = getRepoRoot()

var PROJECT_FOLDER = RepoRoot + "projects/"
