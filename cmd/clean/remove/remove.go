package remove

import (
	"cmd/common"
	"fmt"
	"os/exec"
)

func RemoveGeneratedFiles() {

	filesToRemove := []string{
		fmt.Sprintf("%stest.hdd", common.REPO_ROOT),
		fmt.Sprintf("%sDATAFLS.INF", common.REPO_ROOT),
		fmt.Sprintf("%sDSKIMG.INF", common.REPO_ROOT),
		fmt.Sprintf("%sBOOTX64.EFI", common.REPO_ROOT),
		fmt.Sprintf("%skernel.bin", common.REPO_ROOT),
	}

	exec.Command("rm", append([]string{"-f"}, filesToRemove...)...).Run()

	exec.Command("find", common.REPO_ROOT, "-type", "d", "-name", "build", "-exec", "rm", "-r", "{}", "+").Run()
	exec.Command("find", common.REPO_ROOT, "-type", "d", "-name", ".cache", "-exec", "rm", "-r", "{}", "+").Run()
	exec.Command("find", common.REPO_ROOT, "-type", "f", "-name", "*.dot*", "-exec", "rm", "-r", "{}", "+").Run()
	exec.Command("find", common.REPO_ROOT, "-type", "l", "-name", "compile_commands.json", "-exec", "rm", "-r", "{}").Run()
}
