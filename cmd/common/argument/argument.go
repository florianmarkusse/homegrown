package argument

import (
	"cmd/common"
	"fmt"
	"io"
	"log"
	"os/exec"
	"strings"
)

func AddArgument(builder *strings.Builder, arg string) {
	builder.WriteString(fmt.Sprintf("  %s \\\n", arg))
}

func ExecCommand(command string) {
	ExecCommandWriteError(command)
}

func ExecCommandWriteError(command string, errorWriters ...io.Writer) {
	fmt.Printf("%s%s%s\n", common.BOLD, command, common.RESET)

	cmd := exec.Command("bash", "-c", command)

	cmd.Stdout = log.Writer()
	if len(errorWriters) > 0 {
		cmd.Stderr = io.MultiWriter(append([]io.Writer{log.Writer()}, errorWriters...)...)
	} else {
		cmd.Stderr = log.Writer()
	}

	var err = cmd.Run()
	if err != nil {
		log.Fatalf("Command failed, aborting!")
	}
}
