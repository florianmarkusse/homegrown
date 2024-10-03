package argument

import (
	"cmd/common"
	"fmt"
	"log"
	"os/exec"
	"strings"
)

func AddArgument(builder *strings.Builder, arg string) {
	builder.WriteString(fmt.Sprintf("  %s \\\n", arg))
}

func ExecCommand(command string) error {
	fmt.Printf("%s%s%s\n", common.BOLD, command, common.RESET)

	cmd := exec.Command("bash", "-c", command)
	cmd.Stdout = log.Writer() // Redirect output to log
	cmd.Stderr = log.Writer() // Redirect error output to log
	return cmd.Run()          // Run the command and return the error, if any
}
