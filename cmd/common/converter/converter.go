package converter

import "strings"

func ArrayIntoPrintableString(array []string) string {
	builder := strings.Builder{}
	for i, element := range array {
		builder.WriteString(element)
		if i != len(array)-1 {
			builder.WriteString(" ")
		}
	}

	return builder.String()
}
