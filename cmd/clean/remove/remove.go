package remove

import (
	"cmd/common"
	"fmt"
	"os/exec"
)

func RemoveGeneratedFiles() {
	filesToRemove := []string{
		fmt.Sprintf("%s/%s", common.REPO_ROOT, common.FLOS_EFI_FILE),
		fmt.Sprintf("%s/%s", common.REPO_ROOT, common.FLOS_KERNEL_FILE),
		fmt.Sprintf("%s/%s", common.REPO_ROOT, common.FLOS_UEFI_IMAGE_FILE),
	}

	exec.Command("rm", append([]string{"-f"}, filesToRemove...)...).Run()

	exec.Command("find", common.REPO_ROOT, "-type", "d", "-name", "build", "-exec", "rm", "-r", "{}", "+").Run()
	exec.Command("find", common.REPO_ROOT, "-type", "d", "-name", ".cache", "-exec", "rm", "-r", "{}", "+").Run()
	exec.Command("find", common.REPO_ROOT, "-type", "f", "-name", "*.dot*", "-exec", "rm", "-r", "{}", "+").Run()
	exec.Command("find", common.REPO_ROOT, "-name", "compile_commands.json", "-exec", "rm", "-r", "{}", ";").Run()
}
