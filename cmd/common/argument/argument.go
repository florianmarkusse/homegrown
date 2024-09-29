package argument

import (
	"cmd/common"
	"fmt"
	"strings"

	"github.com/bitfield/script"
)

func AddArgument(builder *strings.Builder, arg string) {
	builder.WriteString(fmt.Sprintf("  %s\n", arg))
}

func RunCommand(executable string, options string) {
	fmt.Printf("%s%s%s\n%s", common.BOLD, executable, common.RESET, options)

	var finalCommand = fmt.Sprintf("%s\n%s", executable, options)
	script.Exec(finalCommand).Stdout()
}
