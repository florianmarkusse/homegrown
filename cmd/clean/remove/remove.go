package remove

import (
	"cmd/common"
	"fmt"
	"os/exec"
)

func RemoveGeneratedFiles() {

	filesToRemove := []string{
		fmt.Sprintf("%stest.hdd", common.RepoRoot),
		fmt.Sprintf("%sDATAFLS.INF", common.RepoRoot),
		fmt.Sprintf("%sDSKIMG.INF", common.RepoRoot),
		fmt.Sprintf("%sBOOTX64.EFI", common.RepoRoot),
		fmt.Sprintf("%skernel.bin", common.RepoRoot),
	}

	exec.Command("rm", append([]string{"-f"}, filesToRemove...)...).Run()

	exec.Command("find", common.RepoRoot, "-type", "d", "-name", "build", "-exec", "rm", "-r", "{}", "+").Run()
	exec.Command("find", common.RepoRoot, "-type", "d", "-name", ".cache", "-exec", "rm", "-r", "{}", "+").Run()
	exec.Command("find", common.RepoRoot, "-type", "f", "-name", "*.dot*", "-exec", "rm", "-r", "{}", "+").Run()
	exec.Command("find", common.RepoRoot, "-type", "l", "-name", "compile_commands.json", "-exec", "rm", "-r", "{}").Run()
}
